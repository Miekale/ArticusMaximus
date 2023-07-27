import matplotlib.pyplot as plt
import numpy as np
import cv2

#Functions:
def perp_distance(pstart, pend, point):

    vectoru = np.array([point[0] - pstart[0], point[1] - pstart[1]])
    vectorv = np.array([pend[0] - pstart[0], pend[1] - pstart[1]])
    
    #finding  projection vector
    v_norm = np.sqrt(vectorv[0]**2 + vectorv[1]**2)
    
    proj_u_on_v = np.array([np.dot(vectoru, vectorv) / v_norm**2 * vectorv[0], np.dot(vectoru, vectorv) / v_norm**2 * vectorv[1]])

    #finding perpendicular vector
    perp_u_on_v = np.array([vectoru[0] - proj_u_on_v[0], vectoru[1] - proj_u_on_v[1]])

    return np.sqrt(perp_u_on_v[0]**2 + perp_u_on_v[1]**2)

def remove_duplicate_points(contour):
    i = 0
    num_deleted = 0
    while i  < contour.shape[0] - 1:
        if np.array_equal(contour[i - num_deleted], contour[i+1 - num_deleted]):
            contour = np.delete(contour, i - num_deleted, 0)
            print(f"point {i - num_deleted} deleted")
            print(contour)
            num_deleted += 1
        i += 1
    return contour

def Douglas_Peucker(points:np.array, epsilon:int):
    #max distance and index of the distance
    dmax = 0
    index = 0

    #for all points find distance and set dmax if largest
    for i, value in enumerate(points):
        distance = perp_distance(points[0], points[len(points)-1], points[i])
        if distance > dmax:
            dmax = distance
            index = i
    result_list = np.array([])

    #recursive algorithm
    if (dmax > epsilon):
        upper_recursive = Douglas_Peucker(points[0:index], epsilon)
        lower_recursive = Douglas_Peucker(points[index:len(points)], epsilon)
        
        #build resulting list
        result_list = np.concatenate((lower_recursive[0:(len(lower_recursive) - 1)], upper_recursive[0:len(upper_recursive - 1)]), axis=0)
    else:
        result_list = np.array([points[0], points[len(points)-1]])

    # return value
    return result_list

def remove_small_contours(contours:list, min_points_thresh:int):
    i = 0
    while i < len(contours):
        if contours[i].shape[0] <= min_points_thresh:
            contours.pop(i)
            print(f'{i} contour dropped')
        i += 1
    return contours

def remove_duplicate_contours(contours, match_thresh, pixel_thresh):
    # Checking contour pairs
    i = 0
    num_deleted = 0
    while(i < len(contours) - 1):
        # print(f'{ i } and {i + 1}: {cv2.matchShapes(contours[i], contours[i+1], cv2.CONTOURS_MATCH_I3,0)}')
        # print(f'{contours[i].mean()} and {contours[i+1].mean()}')
        match_score = cv2.matchShapes(contours[i - num_deleted], contours[i+1 - num_deleted], cv2.CONTOURS_MATCH_I3,0)
        # Remove contours with same shape in same location
        if match_score < match_thresh:
            mean_1 = contours[i - num_deleted].mean()
            mean_2 = contours[i+1 - num_deleted].mean()
            if (abs(mean_1 - mean_2) < pixel_thresh):
                contours.pop(i - num_deleted)
                # print("contour popped")
        i += 1
    
    return contours

def detect_edges(image, blur_kernel, thresh_lower, thresh_upper, aperature_size):
    # Greyscales, gaussian blurs, and performs canny edge detection on image 
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, blur_kernel, sigmaX=0, sigmaY=0)
    canny_blur = cv2.Canny(blur, thresh_lower, thresh_upper, aperature_size)

    return canny_blur 

def resize_image(image, max_width, max_height):
    h, w = image.shape[:2]
    aspect = h/w
    image = cv2.resize(image, (max_width, int(max_width * aspect)), interpolation=cv2.INTER_AREA)
    if image.shape[0] > max_height:
        image = cv2.resize(image, (int(max_height / aspect), max_height) , interpolation=cv2.INTER_AREA)
    
    return image 

# Pre-processing 
image = cv2.imread(r"C:\Users\markd\Documents\GitHub\ArticusMaximus\python_edge_detection\sample_img\sponge.png")
image = resize_image(image, max_width=360, max_height=400)
canny_blur = detect_edges(image, blur_kernel=(5,5), thresh_lower=10, thresh_upper=150, aperature_size=3)


# Find contours from canny blurred image
contours, hierarchy = cv2.findContours(canny_blur, 
    cv2.RETR_TREE , cv2.CHAIN_APPROX_SIMPLE)

# Epsilon value for Douglas Peucker algorithm, bigger == less details
EPSILON = 0.5

# Filter out small contours and duplicate contours
contours = list(contours)
contours = remove_small_contours(contours, 2)
contours = remove_duplicate_contours(contours, 0.05, 10)

# Simpifly all contours
i = 0
while (i < len(contours)):
    # Contours is in [[],] shape, squeeze to simplify 
    contours[i] = np.squeeze(contours[i])
    # Remove any two points that are the same & next to each other
    contours[i] = remove_duplicate_points(contours[i])
    # Douglas Peucker Algorithm
    contours[i] = Douglas_Peucker(contours[i], EPSILON)
    # Close contour (connect back to starting point)
    contours[i][-1] = contours[i][0]
    # Remove any duplicates resulting from Douglas Peucker
    contours[i] = remove_duplicate_points(contours[i])
    i += 1

# Filter again after simplifying 
contours = remove_small_contours(contours, 2)
contours = remove_duplicate_contours(contours, 0.05, 10)

h, w = image.shape[:2]
#Reflecting Image
for contour in range(len(contours)):
    for point in range(len(contours[contour])):
        print(f'len contours: {len(contours)}')
        print(f'len contours[contour]: {len(contours[contour])}')
        print(f'{contours[contour][point]}')
        contours[contour][point] = [contours[contour][point][0] / 2, contours[contour][point][1] / 2]

# Write contours to file
f = open(r"C:\Users\markd\Documents\GitHub\ArticusMaximus\python_edge_detection\contour_output.txt", 'w')
for contour in contours:
    for i in range(len(contour)):
        if i == 0:
            f.write("M ")
        else:
            f.write("D ")
        f.write(str(contour[i, 0]))
        f.write(" ")
        f.write(str(contour[i, 1] + 30))
        f.write(" \n")
f.close()


#Reflecting image vertically
for contour in range(len(contours)):
    for point in range(len(contours[contour])):
        print(f'len contours: {len(contours)}')
        print(f'len contours[contour]: {len(contours[contour])}')
        print(f'{contours[contour][point]}')
        contours[contour][point] = [contours[contour][point][0], abs(h/2 - contours[contour][point][1])]

# Display vertically flipped results 
plt.xlim(0, 200)
plt.ylim(0, 200)
for i in range(len(contours)):
    print(contours[i])
    plt.plot(contours[i][:,0], contours[i][:,1], label=f'{i}')

plt.show()