import cv2
import numpy as np
import matplotlib.pyplot as plt

# Load the image
image = cv2.imread(r"C:\Users\markd\Desktop\Coding\ME 101\EV3Projection\imageToGcode\among-us-original-image.png")

# Resize
image = cv2.resize(image, dsize=[500,500])
# Greyscale
image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
# Threshold
ret, image = cv2.threshold(image,127,255,cv2.THRESH_BINARY)

contours, hierachy = cv2.findContours(image, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_SIMPLE)
cv2.drawContours(image, contours, -1, (0,255,0), 3)

# Write contours to file
f = open("contour_output.txt", "a")
for contour in contours:
    f.write(contour)
f.close()

cv2.imshow('Contours', image)
cv2.waitKey(0)
cv2.destroyAllWindows()

