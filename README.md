# ArticusMaximus

Articus maximus is a sketching robot that takes digital image files (.JPG, .PNG) and draws the equivalent line-art on paper. 
This repo contains all image processing code in Python **(python_edge_detection/image_processing.py)** and the firmware code in C **(robot_c/PID)**.

## Python Image Processing

In general, the line-art generation can be seperated into 2 distinct parts: 
1. Pre-contour detection
2. Contour post-processing

#### Pre-contour detection
This part is where we turn an input image to into a list of contours. A contour is just a set of points that when joined together create a closed shape. 
First, we do the required pre-processing (scaling to desired size, blurring, and greyscaling) and then ran this image array through Canny edge detection.
The edge detection output is then ran through OpenCV's contour detection algorithm to get the list of contours mentionned above.

#### Contour Post Processing
The second part is the most vital part of the process. The contour detection algorithm output can not be drawn because it contains many invalid and duplicate contours,
as the contour detection algorithm picks up all the details coming in from edge detection.

We do several filtering steps on this contour list to remove undesirable contours:
1. Removing similar contours
2. Removing small contours

We also modify each contours themselves, to speed up the robot drawing speed:
1. Removing duplicate points
2. Contour simplification

## C Firmware

Our robot uses a 2-aix gantry system for movement in the x and y-axis, with each axis powered by one motor. 
We programmed 2 different methods of control:

1. Open-loop control
The logic for open-loop control takes an input ending position and measures the starting position to calculate the motor powers required. It then power each motor at
their respective power levels until the current position is within tolerance for both x and y. 

3. Closed-loop PID control
We programmed a PID controller with a lower pass filter for the D term, anti-windup, and a 1D motion profile so that the robot can draw diagonal lines accurately.
The closed-loop control program has better drawing accuracy compared to the open-loop control, and by tuning the 'speed' parameter we can also control the robot's
speed to accuracy tradeoff for each drawing.

## What We Learned
1. Hardware integration is HARD. Even with software unit testing, small integration errors that were not caught during development ended up costing us many hours to fix. This stresses the need for extensive unit testing before integrating all systems.
2. The mechanical systems of a machine are vital to its success, especially in regards to picking the correct materials for each task(Never choose LEGO). 
3. I hate the ROBOTC IDE I hate the ROBOTC IDE I hate the ROBOTC IDE I hate the ROBOTC IDE I hate the ROBOTC IDE I hate the ROBOTC IDE
