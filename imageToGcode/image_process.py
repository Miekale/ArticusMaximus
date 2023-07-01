import cv2
from pathlib import Path
import math
import numpy as np
from PIL import Image, ImageTk



filteredContoursGlob = None
filteredGCODEContours = None
originalSize = None

# The uploaded images gets upscaled to make more accurate contours
image_scale_new_width = 2640

# When create SVG the image gets downscaled by this factor  # new size = image_scale_new_width / image_to_svg_size_reduction
image_to_svg_size_reduction = 2


# Basic image resizer function
def resizeMyImage(img, max_width, max_height):
	inimg = img
	#maxWidth = 132
	maxWidth = max_width

	width = inimg.shape[1]
	scaleFactor =  maxWidth / inimg.shape[1]

	newwidth = round(inimg.shape[1] * scaleFactor)
	newheight = round(inimg.shape[0] * scaleFactor)
	dim = (newwidth, newheight)

	resizedImg = cv2.resize(inimg, dim, interpolation = cv2.INTER_AREA)

	if resizedImg.shape[0] > max_height:
		scaleFactor = max_height / resizedImg.shape[0]
		newwidth = round(resizedImg.shape[1] * scaleFactor)
		
		newheight = round(resizedImg.shape[0] * scaleFactor)
		dim = (newwidth, newheight)
		resizedImg = cv2.resize(resizedImg, dim, interpolation = cv2.INTER_AREA)

	return resizedImg


# This function does basically everything from reading in the to convert them into contours
def createPreviewImages(filename, thresh_min, thresh_max, isInv, blur_mode, min_contour_area, max_contour_area, contour_threshold, horizontal_mirror, vertical_mirror):

	# Reading the uploaded image	
	img = cv2.imread(filename)
	img_copy = img.copy()

	# Mirroring the image if mirror chechboxes are on
	if horizontal_mirror:
		img_copy = cv2.flip(img_copy, 0) # Flip image back to normal

	if vertical_mirror:
		img_copy = cv2.flip(img_copy, 1) # Flip image back to normal
	


	# Upscaling the image for better contour detection
	global image_scale_new_width
	img_copy = resizeMyImage(img_copy, image_scale_new_width, 999999)

	
	# Creating a blank canvas to draw the contours on
	canvas = np.zeros(img_copy.shape, np.uint8)

	# Creating a smaller version of the image for preview 
	img_copy_show = resizeMyImage(img_copy, 330, 435)

	# The image that gets process for the final contours (e.g SVG and G-code creating) isn't downscaled
	img_copy_process = img_copy

	# Making the grey filtered image
	gray_show = cv2.cvtColor(img_copy_show,cv2.COLOR_BGR2GRAY)
	gray_process = cv2.cvtColor(img_copy_process,cv2.COLOR_BGR2GRAY)


	# Making the bluredd images based on the user's choice
	if blur_mode == 'Gaussian Blur':
		blurred_show = cv2.GaussianBlur(gray_show, (5,5),0)
		blurred_process = cv2.GaussianBlur(gray_process, (5,5),0)
	elif blur_mode == '2D Filter':
		kernel = np.ones((5,5),np.float32)/25
		blurred_show = cv2.filter2D(gray_show,-1,kernel)
		blurred_process = cv2.filter2D(gray_process,-1,kernel)
	elif blur_mode == "Median Blur":
		blurred_show = cv2.medianBlur(gray_show, 1)
		blurred_process = cv2.medianBlur(gray_process, 1)
	elif blur_mode == 'Bilateral Filter':
		blurred_show = cv2.bilateralFilter(gray_show,9,75,75)
		blurred_process = cv2.bilateralFilter(gray_process,9,75,75)
	elif blur_mode == 'Laplacian':
		blurred_show = cv2.Laplacian(gray_show, cv2.CV_16S, ksize=3)
		blurred_show = cv2.convertScaleAbs(blurred_show)
		blurred_process = cv2.Laplacian(gray_process, cv2.CV_16S, ksize=3)
		blurred_process = cv2.convertScaleAbs(blurred_process)
	elif blur_mode == 'Simple Blur':
		blurred_show = cv2.blur(gray_show,(5,5))
		blurred_process = cv2.blur(gray_process,(5,5))
	else:
		blurred_show = gray_show
		blurred_process = gray_process		


	# Detection the contours and drawing them on the canvas # they also get filtered by area if 'Contour area filter' is on
	contours = detectContours(blurred_process, isInv, thresh_min, thresh_max, False)
	displayContours, max_area, min_area, filteredContours = drawContoursOnCanvas(canvas, contours, min_contour_area, max_contour_area, contour_threshold)

	# Since my Ender-3 draws the images upside down, for the G-code creation the the images gets flip before contour detection, so the
	# printer draws it in the correct orientation # they also get filtered by area if 'Contour area filter' is on
	forGODEcontours = detectContours(blurred_process, isInv, thresh_min, thresh_max, True)
	filteredContoursGCODE = filterContours(forGODEcontours, min_contour_area, max_contour_area, contour_threshold)

	# Setting the global variables of the created images and contours
	global filteredContoursGlob
	filteredContoursGlob = filteredContours
	global filteredGCODEContours
	filteredGCODEContours = filteredContoursGCODE
	global originalSize
	originalSize = [img_copy.shape[1], img_copy.shape[0]]


	# The canvas with the contours drawn on it gets downscaled as well, to match the other preview images
	resizedCanvas = resizeMyImage(displayContours, 330, 435) 
	
	# Image is too dark after size reduction, this makes it brighter
	resizedCanvas = cv2.convertScaleAbs(resizedCanvas, 0, 5) 

	# This is the images that is shown when the users zooms to preview image
	resizedCanvasZoom = resizeMyImage(displayContours, 660, 800)
	resizedCanvasZoom = cv2.convertScaleAbs(resizedCanvasZoom, 0, 5)


	# The openCV images are converted to tKinter images
	b,g,r = cv2.split(img_copy_show)
	tkimg = cv2.merge((r,g,b))
	tkim = Image.fromarray(tkimg)
	rotated_1_tkim = tkim.transpose(Image.FLIP_LEFT_RIGHT)
	rotated_2_tkim = rotated_1_tkim.transpose(Image.FLIP_TOP_BOTTOM)
	imgtk_ori = ImageTk.PhotoImage(image=tkim)

	#b,g,r = cv2.split(gray)
	#tkimg = cv2.merge((r,g,b))
	tkim = Image.fromarray(gray_show)
	rotated_1_tkim = tkim.transpose(Image.FLIP_LEFT_RIGHT)
	rotated_2_tkim = rotated_1_tkim.transpose(Image.FLIP_TOP_BOTTOM)
	imgtk_gray = ImageTk.PhotoImage(image=tkim)

	#b,g,r = cv2.split(blurred)
	#tkimg = cv2.merge((r,g,b))
	tkim = Image.fromarray(blurred_show)
	rotated_1_tkim = tkim.transpose(Image.FLIP_LEFT_RIGHT)
	rotated_2_tkim = rotated_1_tkim.transpose(Image.FLIP_TOP_BOTTOM)
	imgtk_blurred = ImageTk.PhotoImage(image=tkim)

	
	# Convert OpenCV canvas image to tKinter image 
	tkim = Image.fromarray(resizedCanvas)
	rotated_1_tkim = tkim.transpose(Image.FLIP_LEFT_RIGHT)
	rotated_2_tkim = rotated_1_tkim.transpose(Image.FLIP_TOP_BOTTOM)
	imgtk_contour = ImageTk.PhotoImage(image=tkim)
	#tkImage_contour = imgtk_contour

	tkim = Image.fromarray(resizedCanvasZoom)
	rotated_1_tkim = tkim.transpose(Image.FLIP_LEFT_RIGHT)
	rotated_2_tkim = rotated_1_tkim.transpose(Image.FLIP_TOP_BOTTOM)
	imgtk_contour_zoom = ImageTk.PhotoImage(image=tkim)
	#tkImage_contour_zoom = imgtk_contour_zoom

	return imgtk_ori, imgtk_gray, imgtk_blurred, imgtk_contour, imgtk_contour_zoom, max_area, min_area, displayContours, filteredContours


# Creating the SVG file from the contours
def createSVG(image_name):

	returnBool = False
	try:
		global filteredContoursGlob
		global originalSize
		global image_to_svg_size_reduction

		width = originalSize[0]
		height = originalSize[1]

		Path("output").mkdir(parents=True, exist_ok=True)

		f = open(f"output/{image_name}.svg", "w+")
		f.write(f'<svg width="{width / image_to_svg_size_reduction}" height="{height / image_to_svg_size_reduction}" style="background-color:white" xmlns="http://www.w3.org/2000/svg">')

		for c in filteredContoursGlob:
			f.write('<path fill="none" d="M')
			for i in range(len(c)):
				x, y = c[i][0] / 2
				f.write(f"{x} {y} ,")
			f.write('" style="stroke:black"/>')
		f.write("</svg>")
		returnBool = True
	except Exception as e:
		print(e)
		returnBool=False


	return returnBool


# Running the openCV findContour function
def detectContours(image, isInv,threshold_min, threshold_max, forGCODE):

	# When function called for G-code creation, the image gets flipped for correct orientation
	if forGCODE:
		image = cv2.flip(image, 0)
		#image = cv2.flip(image, 1)

	# THRESH_BINARY or THRESH_BINARY_INV threshold depending on the user's choice
	if isInv:
		ret, binary = cv2.threshold(image,threshold_min,threshold_max,cv2.THRESH_BINARY_INV)
	else:
		ret, binary = cv2.threshold(image,threshold_min,threshold_max,cv2.THRESH_BINARY)

	contours, hierarchy = cv2.findContours(binary,cv2.RETR_TREE,cv2.CHAIN_APPROX_NONE)
	
	return contours 


# Drawing the detected contours on the canvas
def drawContoursOnCanvas(canvas, contours, min_contour_area, max_contour_area, contour_threshold):
	areaList = []
	filteredContours = []
	for cnt in contours :
		
		cntArea= cv2.contourArea(cnt)
		areaList.append(cntArea)


		# If 'Contour Area Filter' is on, only the correct contours are drawn
		if contour_threshold:
			if cntArea >= min_contour_area-10 and cntArea <= max_contour_area+10:
				approx = cv2.approxPolyDP(cnt, 0.0001 * cv2.arcLength(cnt, True), False)
				filteredContours.append(approx)
				cv2.drawContours(canvas, [approx], -1, (255, 0, 0), 1)

		# If 'Contour Area Filter' is off, every contour is drawn
		else:
			approx = cv2.approxPolyDP(cnt, 0.0001 * cv2.arcLength(cnt, True), False)
			cv2.drawContours(canvas, [approx], -1, (255, 0, 0), 1)
			filteredContours.append(approx)
	    
	# Finding the biggest and smallest contour area for the contour area slider endpoints	
	max_area = max(areaList)
	min_area = min(areaList)

	return canvas, max_area, min_area, filteredContours


# If the contour detection is done for the G-code creation, the contours aren't drawn on any canvas, so they go through a separate filter function     
def filterContours(gcodeContours, min_contour_area, max_contour_area, contour_threshold):
	filteredContours = []

	for cnt in gcodeContours :
		
		cntArea= cv2.contourArea(cnt)

		if contour_threshold:
			if cntArea >= min_contour_area-10 and cntArea <= max_contour_area+10:
				approx = cv2.approxPolyDP(cnt, 0.0001 * cv2.arcLength(cnt, False), False)
				filteredContours.append(approx)
		else:
			approx = cv2.approxPolyDP(cnt, 0.0001 * cv2.arcLength(cnt, False), False)
			
			filteredContours.append(approx)

	return filteredContours	


# Generating the G-code
def generateGcode(contours, image, printer_area, printer_z_heights, printing_speed, multiPassArray):
	
	# Calculating a down scaling value, to make to image fit the printing area set by the user	
	x_scaler = image.shape[1] / printer_area[2]
	y_scaler = image.shape[0] / printer_area[3]
	scaler = max(x_scaler, y_scaler)

	left_x = printer_area[0]
	left_y = printer_area[1]

	work_height = printer_z_heights[0]
	lift_height = printer_z_heights[1]
	safe_height = printer_z_heights[2]


	# Checking if multi pass/layer function is on
	isMultiPass = multiPassArray[0]
	if isMultiPass:
		multiPassNum = multiPassArray[1]
		multiPassHeight = multiPassArray[2]
	else:
		multiPassNum = 1
		multiPassHeight = 0

	# This is useless but I was lazy the change the 'contours' variable everywhere
	global filteredGCODEContours
	contours = filteredGCODEContours


	# Basic settings for the 3D printer
	gcode = "G00 S1; endstops\nG00 E0; no extrusion\nG01 S1; endstops\nG01 E0; no extrusion\nG21; millimeters\nG90; absolute\n"

	# Homing the printer head
	gcode = gcode + "G28 X; home\nG28 Y; home\nG28 Z; home\n"

	# Lifting the printer head to the Safe height
	gcode = gcode + f"G00 F300.0 Z{safe_height}; \n"

	# Printer head goes to the left bottom of the given printer area
	gcode = gcode + f"G00 F2400.0 Y{left_y};\n"
	gcode = gcode + f"G00 F2400.0 X{left_x};\n"

	# Printer goes to the image's first point
	gcode = gcode + f"G00 F2400.0 X{contours[0][0][0][0]/scaler + left_x} Y{contours[0][0][0][1]/scaler + left_y};\n"

	# Printer head is lowered to the Work height	
	gcode = gcode + f"G00 F300.0 Z{work_height};\n"


	# Repeating for the amount of layers
	for mp in range(multiPassNum):
		currentPass = mp
		isPenDown = False

		gcode = gcode + f"\n (-----------------Layer {currentPass+1} -----------------) \n"
		for contour in contours:
			i = 0

			while i < len(contour):
				x = contour[i][0][0] / scaler	
				y = contour[i][0][1] / scaler

				if i > 0 and not isPenDown:
					gcode = gcode + f"G00 F300.0 Z{work_height - currentPass * multiPassHeight};\n" #Put pet down
					isPenDown = True # put pen down
				
				code = f"G00 F{printing_speed} X{x + left_x} Y{y + left_y};\n" # Drawing
				gcode = gcode + code
				i=i+1

			gcode = gcode +	f"G00 F300.0 Z{lift_height - currentPass * multiPassHeight};\n" # Lift pen up
			isPenDown = False # Lift pen

	gcode = gcode + f"G00 F300.0 Z{safe_height}"



	return gcode

# Testing out functions
if __name__ == "__main__":
	pass
