import matplotlib.pyplot as plt
import numpy as np
import cv2

### IMAGE PROCESSING FUNCTIONS ###
def perp_distance(line_start: np.ndarray,
                  line_end: np.ndarray,
                  point: np.ndarray) \
    -> float:
    """
    Calculates perpendicular distance between point and a line segment.

    PARAMETERS
    ----------
    line_start: np.ndarray
        Point at where the line segment starts.

    line_end: np.ndarray
        Point at where the line segment ends.

    point: np.ndarray
        Point to calculate perpendicular distance to the line segment.

    RETURNS
    -------
    perp_norm: float
        Perpendicular distance.

    """
    vectoru = np.array([point[0] - line_start[0], point[1] - line_start[1]])
    vectorv = np.array([line_end[0] - line_start[0], line_end[1] - line_start[1]])
    
    # Finding  projection vector
    v_norm = np.sqrt(vectorv[0]**2 + vectorv[1]**2)
    proj_u_on_v = np.array(
        [np.dot(vectoru, vectorv) / v_norm**2 * vectorv[0],
         np.dot(vectoru, vectorv) / v_norm**2 * vectorv[1]]
    )

    # Finding perline_endicular vector
    perp_u_on_v = np.array(
        [vectoru[0] - proj_u_on_v[0],
        vectoru[1] - proj_u_on_v[1]]
    )

    perp_norm = np.sqrt(perp_u_on_v[0]**2 + perp_u_on_v[1]**2)

    return perp_norm

def remove_duplicate_points(contour: np.ndarray) -> np.ndarray:
    """
    Removes duplicated points from a contour. Duplicate points are defined as
    two or more consecutive points with the same position.

    PARAMETERS
    ----------
    contour: np.ndarray
        Array containing points that make up the contour.

    RETURNS
    -------
    contour: np.ndarray
        Contour after duplicates removed.

    """
    i = 0
    num_deleted = 0
    while i  < contour.shape[0] - 1:
        if np.array_equal(contour[i - num_deleted], contour[i+1 - num_deleted]):
            contour = np.delete(contour, i - num_deleted, 0)
            num_deleted += 1
        i += 1

    return contour

def Douglas_Peucker(points: np.ndarray, epsilon:float) -> np.ndarray:
    """
    Applies the Douglas_Peuker simplification algorithm to a list of points.

    PARMETERS
    ---------
    points: np.ndarray
        Array containing points for simplification.

    epsilon: float
        Determines the distance points can be from the secant line
        Manipulating this variable changes the magnitude of point reduction

    RETURNS
    -------
    result_array: np.ndarray
        Array of points after simplifying.

    """
    # Max distance and index of the distance
    max_dist = 0
    index = 0

    for i, _ in enumerate(points):
        # Get distance
        distance = perp_distance(points[0], points[len(points)-1], points[i])

        # Update max_dist
        if distance > max_dist:
            max_dist = distance
            index = i

    result_array = np.array([])

    # Recursive case
    if max_dist > epsilon:
        upper_recursive = Douglas_Peucker(points[0:index], epsilon)
        lower_recursive = Douglas_Peucker(points[index:len(points)], epsilon)
        
        # Build resulting list
        result_array = np.concatenate(
            tuple(
                lower_recursive[0:(len(lower_recursive) - 1)],
                upper_recursive[0:len(upper_recursive - 1)],
            ),
            axis=0,
        )

    else:
        result_array = np.array([points[0], points[len(points)-1]])

    return result_array

def remove_small_contours(contours:"list[np.ndarray]", min_points_thresh:int) \
    -> "list[np.ndarray]":

    """
    Removes contours under threshold size from a contour list.

    PARAMETERS
    ----------
    contours: "list[np.ndarray]"
        List of contours.

    RETURNS
    -------
    contours: "list[np.ndarray]"
        List of contours after small contour removal.

    """
    i = 0
    while i < len(contours):
        if contours[i].shape[0] <= min_points_thresh:
            contours.pop(i)
        i += 1

    return contours

def remove_similar_contours(contours: "list[np.ndarray]",
                            shape_thresh: float,
                            position_thresh: float) \
    -> "list[np.ndarray]":
    """
    Remove similar contours from a list of contours.

    PARAMETERS
    ----------
    contours: "list[np.ndarray]"
        List of contours.

    shape_thresh: float
        A threshold for shape similarity. Higher values result in more tolerance
        for two contours to be considered as the same shape for removal, and
        vice versa.
    
    position_thresh: float
        A threshold for positional similarity. Higher values result in more
        tolerance for two contours to be considered in the same position, and
        vice versa.

    RETURNS
    -------
    contours: "list[np.ndarray]"
        List of contours after similar contour removal.


    """
    # Checking contour pairs
    i = 0
    num_deleted = 0
    while(i < len(contours) - 1):

        # Get shape matching score
        match_score = cv2.matchShapes(
            contours[i - num_deleted],
            contours[i+1 - num_deleted],
            method=cv2.CONTOURS_MATCH_I3,
            parameter=0,
        )

        # Remove contours with same shape in same location
        if match_score < shape_thresh:
            mean_1 = contours[i - num_deleted].mean()
            mean_2 = contours[i+1 - num_deleted].mean()

            # Compare pixel location
            if (abs(mean_1 - mean_2) < position_thresh):
                contours.pop(i - num_deleted)

        i += 1
    
    return contours

def detect_edges(image, blur_kernel, thresh_lower, thresh_upper, aperature_size) \
    -> np.ndarray:
    """
    Performs required pre-processing steps and then performs edge detection on
    an image.

    PARAMETERS
    ----------
    image: np.ndarray
        Image for edge detection.
    
    blur_kernel: tuple(int, int)
        Kernel size of Gaussian blurring.
    
    thresh_lower: float
        Lower threshold for Canny edge detection.

    thresh_upper: float
        Upper threshold for Canny edge detection.

    aparature_size: int
        Aparature size for Canny edge detection.

    RETURNS
    -------
    canny: np.ndarray
        Edged image array.

    """
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, blur_kernel, sigmaX=0, sigmaY=0)
    canny = cv2.Canny(blur, thresh_lower, thresh_upper, aperature_size)

    return canny 

def resize_image(image: np.ndarray,
                 max_width: int,
                 max_height: int) \
    -> np.ndarray:
    """
    Resize image to a max width and height.

    PARAMETERS
    ----------
    image: np.ndarray
        Image to resize.

    max_width: int
        Max image widght (pixel) to resize down to.
    
    max_height: int
        Max image height (pixel) to resize down to.

    RETURNS
    -------
    resized_image: np.ndarray
        Resized image.

    """
    # Get current height and width
    h, w = image.shape[:2]

    # Calculate aspect ratio
    aspect = h/w

    # Resize
    resized_image = cv2.resize(
        image,
        tuple(
            max_width,
            int(max_width * aspect)
        ),
        interpolation=cv2.INTER_AREA,
    )

    if image.shape[0] > max_height:
        resized_imge = cv2.resize(
            image,
            tuple(
                int(max_height / aspect),
                max_height
            ),
            interpolation=cv2.INTER_AREA)
    
    return resized_image 

### MAIN ###

# Pre-processing 
image = cv2.imread(r"\sample_img\birb.png")
image = resize_image(image, max_width=360, max_height=400)
edged_image = detect_edges(
    image,
    blur_kernel=(5,5),
    thresh_lower=40,
    thresh_upper=250,
    aperature_size=3,
)

# Find contours from canny blurred image
contours, _ = cv2.findContours(
    edged_image,
    cv2.RETR_TREE,
    cv2.CHAIN_APPROX_SIMPLE,
)

# Epsilon value for Douglas Peucker algorithm, bigger == less details
EPSILON = 0.5

# Filter out small contours and duplicate contours
contours = list(contours)
contours = remove_small_contours(contours, 2)
contours = remove_similar_contours(contours, 0.05, 10)

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
contours = remove_similar_contours(contours, 0.05, 10)

# Get height and width
h, w = image.shape[:2]

#Reflecting Image
for contour in range(len(contours)):
    for point in range(len(contours[contour])):
        contours[contour][point] = [contours[contour][point][0] / 2, contours[contour][point][1] / 2]

# Write contours to file
f = open(r"\output_files\bird.txt", 'w')
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
        contours[contour][point] = [contours[contour][point][0], abs(h/2 - contours[contour][point][1])]

# Display vertically flipped results 
plt.xlim(0, 200)
plt.ylim(0, 200)
for i in range(len(contours)):
    print(contours[i])
    plt.plot(contours[i][:,0], contours[i][:,1], label=f'{i}')

plt.show()
