import pkg_resources
import subprocess
import sys
import os

REQUIRED = {
  'tk', 'opencv-python', 'numpy', 'pathlib', 
  'Pillow'
}

installed = {pkg.key for pkg in pkg_resources.working_set}
missing = REQUIRED - installed

# Automatically installing missing packages

# Comment these out if want to install them manually

# if missing:
#     python = sys.executable
#     subprocess.check_call([python, '-m', 'pip', 'install', *missing], stdout=subprocess.DEVNULL)




from tkinter import *    # from tkinter import Tk for Python 3.x
from tkinter.filedialog import askopenfilename
from tkinter import messagebox as MB
import cv2
from pathlib import Path
import math
import numpy as np
from PIL import Image, ImageTk
import image_process as IP
from tkinter import ttk
from tkinter import scrolledtext


#------------- Main window creation ----------------

root = Tk()
root.title('G-code from image')
tabControl = ttk.Notebook(root)

tab1 = ttk.Frame(tabControl)
tab2 = ttk.Frame(tabControl)
tab3 = ttk.Frame(tabControl)

tabControl.add(tab1, text="Image upload")
tabControl.add(tab2, text="G-code generate")
#tabControl.add(tab3, text="Help")

tabControl.pack(expand=1, fill="both")
root.geometry('1020x900')

#-------------Contour detection variables ---------------

image_path = StringVar()
image_name = StringVar()
gcode_var = StringVar()
thresh_min = IntVar()
thresh_min.set(60)
thresh_max = IntVar()
thresh_max.set(255)
inv_contour = BooleanVar()
choosen_blur = StringVar()
choices = ['Gaussian Blur', 'Simple Blur', 'Median Blur', 'Bilateral Filter', '2D Filter', 'Laplacian', 'No Blur']
choosen_blur.set(choices[0])
max_image_area = IntVar()
min_image_area = IntVar()
conture_threshold = BooleanVar()
min_contour_area = IntVar()
max_contour_area = IntVar()
vertical_mirror = BooleanVar()
vertical_mirror.set(False)
horizontal_mirror = BooleanVar()
horizontal_mirror.set(False)


#---------- Printer setting variable ------------------

safe_height = StringVar()
work_height = StringVar()
lift_height = StringVar()
left_x = StringVar()
left_y = StringVar()
right_x = StringVar()
right_y = StringVar()
optionArray = []
hasSettings = BooleanVar()
hasSettings.set(False)
gcodeHasGenerated = BooleanVar()
gcodeHasGenerated.set(False)
printing_speed = IntVar()
printing_speed.set(1500)
isMultiplePasses = BooleanVar()
isMultiplePasses.set(False)
multiplePassesNum = StringVar()
multiplePassesNum.set('1')
multiplePassesNumHeight = StringVar()
multiplePassesNumHeight.set('0')

#------------- Image variables --------------------

preparedImage = None
blurred_process = None
blurred_show = None
contourCanvas = None
tkImageOri = None
tkImageGray = None
tkImageBlurred = None
tkImageContour = None
tkImageContourZoom = None
final_contours = None
isFirstRun=True
cv2Image = None
filteredContourArray = None



# ---------------- Functions -----------------------


# Reading printer settings (e.g  work height, bottom left coord...)
def readPrinterSettings():

	hasSettings.set(False)
	global optionArray

	try:
		options_file = open('settings/options.txt', 'r')
	except:
		#print("No option.txt file yet")
		#MB.showinfo(title="Information", message="You haven't saved your printing parameters, you have add them on the 'G-code generate' tab, to be able to generate G-code!")
		return


	options = options_file.readlines()
	options_file.close()
	option_list = []
	for option in options:
		option = option.strip()
		option_list.append(option)

	if len(option_list) < 7:
		MB.showwarning(title="Warning", message="Your printer settings have missing value(s), please define and save your settings!")
		return
		
	option_check_num = 0

	try: 
		work_height.set(float(option_list[0]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		lift_height.set(float(option_list[1]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		safe_height.set(float(option_list[2]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		left_x.set(float(option_list[3]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		left_y.set(float(option_list[4]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		right_x.set(float(option_list[5]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	try: 
		right_y.set(float(option_list[6]))
		option_check_num = option_check_num + 1 
	except:
		print("readerror")

	#print(option_check_num)

	if option_check_num == 7:
		hasSettings.set(True)
		optionArray = option_list 
	else:
		hasSettings.set(False)
		optionArray = option_list

# Saves the printer settings (e.g work height, bottom left coord...)
def savePrinterSettings():

	global safe_height
	global work_height
	global lift_height
	global left_x
	global left_y
	global right_x
	global right_y

	try:
		safe_height.set(float(safe_height_entry.get()))
		work_height.set(float(work_height_entry.get()))
		lift_height.set(float(lift_height_entry.get()))
		left_x.set(float(left_x_entry.get()))
		left_y.set(float(left_y_entry.get()))
		right_x.set(float(right_x_entry.get()))
		right_y.set(float(right_y_entry.get()))


	except Exception as e: 
		MB.showerror(title="Error", message="Some of the values are incorrect, input only numbers")
		print(e)
		return

	Path("settings").mkdir(parents=True, exist_ok=True)
	f = open('settings/options.txt', 'w')
	f.write('%s\n' % work_height.get())
	f.write('%s\n' % lift_height.get())
	f.write('%s\n' % safe_height.get())
	f.write('%s\n' % left_x.get())
	f.write('%s\n' % left_y.get())
	f.write('%s\n' % right_x.get())
	f.write('%s\n' % right_y.get())
	f.close()


	MB.showinfo(title="Success", message="Your settings were saved successfully")
	
	readPrinterSettings()

# Start when clicking Upload image or New Image buttons, let's you choose a file and reads it
def uploadImage():

	# Open file browser
	filepath = askopenfilename(title="Select a image to analyze")
	path = Path(filepath)
	image_name.set(path.stem)

	
	# Set global variable: Filename
	image_path.set(filepath)

	print(image_name.get())

	# Prepare image for edge detection

	if filepath == "":
		print("No image selected")
		return None

	
	# Refresh label widgets
	setImages()
	
# Creates the different images types from the uploaded image
def createContoures(isInv,thresh_min):
	global preparedImage
	global blurred_process
	global blurred_show
	global contourCanvas
	global tkImageContour
	global final_contours

	if blurred_process is None:
		return None
	
	#global thres_min
	# Big image find contours	
	thresh_min = thres_min_scale.get()
	contours_big = IP.detectContours(blurred_process, isInv,thresh_min)

	if contours_big != None:
		gcode_gen_button.configure(state="active")
		final_contours = contours_big

	# Small image find contours and draw
	#contours_show = IP.detectContours(blurred_process)
	contourCanvas = np.zeros(contourCanvas.shape, np.uint8)
	contourImage, approx = IP.drawContoursOnCanvas(contourCanvas, contours_big)

	# Turn process image into showable image
	#resizedCanvas = IP.resizeMyImage(contourImage, 330) # Resize image to match the other images
	#resizedCanvas = max_contour_area = IntVar()cv2.flip(resizedCanvas, 1) # Flip image back to normal
	#resizedCanvas = cv2.convertScaleAbs(resizedCanvas, 0, 5) # Image is too dark after size reduction, this makes it brighter

	# Convert OpenCV canvas image to tKinter image 
	tkim = Image.fromarray(resizedCanvas)
	imgtk_contour = ImageTk.PhotoImage(image=tkim)
	tkImageContour = imgtk_contour

# Sets the global image variable to the previously created images, and show them on the UI
def setImages():

	global isFirstRun
	try:
		tkimage_ori, tkimage_gray, tkimage_blurred, tkimage_contour, tkimage_contour_zoom, max_area, min_area, cv2image, filteredContours = IP.createPreviewImages(image_path.get(), thresh_min.get(), thresh_max.get(), inv_contour.get(), choosen_blur.get(), min_contour_area.get(), max_contour_area.get(), conture_threshold.get(), horizontal_mirror.get(), vertical_mirror.get())
	except Exception as e: 
		MB.showerror(title="Error", message="Image format is not supported")
		print(e)
		return
		
	im_upload_button.place_forget()	
	max_image_area.set(round(max_area))
	min_image_area.set(round(min_area))

	area_thresh_min_scale.configure(from_=min_area,to=max_area, variable=min_contour_area)
	area_thresh_max_scale.configure(from_=min_area,to=max_area, variable=max_contour_area)

	if isFirstRun:
		area_thresh_max_scale.set(max_image_area.get())
		isFirstRun = False

	area_thresh_frame.grid(row=4, column=0)

	global contourCanvas
	contourCanvas = tkimage_contour

	global tkImageOri
	tkImageOri = tkimage_ori

	global tkImageGray
	tkImageGray = tkimage_gray

	global tkImageBlurred
	tkImageBlurred = tkimage_blurred

	global tkImageContour
	tkImageContour = tkimage_contour

	global tkImageContourZoom
	tkImageContourZoom = tkimage_contour_zoom

	global cv2Image
	cv2Image = cv2image

	global filteredContourArray
	filteredContourArray = filteredContours

	# Update image labels with global variables
	
	im_ori_show_label.configure(image=tkImageOri)
	
	im_gray_show_label.configure(image=tkImageGray)
	
	im_blurred_show_label.configure(image=tkImageBlurred)
	
	im_contour_show_label.configure(image=tkImageContour)

	im_zoom_label.configure(image=tkImageContourZoom)

	all_contour_settings_frame.grid(row=0, column=1, padx=10)

# Starts the G-code creating process
def createGcode():

	global left_x
	global left_y
	global right_x
	global right_y
	global work_height
	global lift_height
	global safe_height
	global cv2Image
	global filteredContourArray
	global printing_speed
	global isMultiplePasses
	global multiplePassesNum
	global multiplePassesNumHeight

	if filteredContourArray is None:
		MB.showerror(title="Error", message="Load an image before generating G-code")
		return

	if not hasSettings.get():
		MB.showerror(title="Error", message="Your printer settings have missing value(s), please define and save your settings!")
		return

	max_x_distance = float(right_x.get()) - float(left_x.get())
	max_y_distance = float(right_y.get()) - float(left_y.get())
	printer_area_dim = [float(left_x.get()), float(left_y.get()), max_x_distance, max_y_distance]
	printer_z_heights = [float(work_height.get()), float(lift_height.get()), float(safe_height.get())]

	if max_x_distance <= 0 or max_y_distance <= 0:
		MB.showinfo(title="How", message=f"Your printing area is really {max_x_distance} x {max_y_distance} mm? How's that possible?")
		return


	if isMultiplePasses:
		try:
			multiplePassesArray = [isMultiplePasses.get(), int(multiplePassesNum.get()), float(multiplePassesNumHeight.get())]
		except Exception as e:
			print(e)
			MB.showerror(title="Error", message="Enter valid 'Multiple passes' parameters")
			return
	else:
		multiplePassesArray = [False, 1, 0]
	
	if multiplePassesArray[1] < 1:
		MB.showinfo(title="Why", message=f"What's the points of making {multiplePassesArray[1]} passes?")
		return

	gcode = IP.generateGcode(filteredContourArray, cv2Image,printer_area_dim, printer_z_heights, printing_speed.get(), multiplePassesArray)
	gcode_var.set(gcode)
	gcodeHasGenerated.set(True)
	gcode_gen_save_button.configure(state="active")
	gcode_text_area.delete("1.0", "end")
	gcode_text_area.insert(INSERT, gcode_var.get())

# Save the G-code to a file
def saveGcodeFile():

	if gcodeHasGenerated.get():
		try:
			Path("output").mkdir(parents=True, exist_ok=True)
			f = open( "output/" + image_name.get() + '.gcode', 'w')
			f.write('%s\n' % gcode_var.get())
			f.close()
			MB.showinfo(title="Success", message="G-Code file has been created and saved in the 'output' directory")
		except:
			MB.showerror(title="Error", message="Saving the G-code file was unsuccessful, an error occured .")
	else:
		MB.showerror(title="Error", message="Generate G-code before trying to save to file")
		return

# When 'Contour area filter' is off, this hides the corresponding sliders
def hideContourThreshold():
	if conture_threshold.get():
		area_thresh_frame.configure(height=140)
		area_thresh_min_scale.grid(row=1, column=0)
		area_thresh_max_scale.grid(row=2, column=0)
		
	else:
		
		area_thresh_min_scale.grid_forget()
		area_thresh_max_scale.grid_forget()
		area_thresh_frame.configure(height=40)

# When 'Multiple passes, layers' is off, this hides the corresponding Entry widgets
def hideMultiplePasses():
	if isMultiplePasses.get():
		printing_parameters_frame.configure(height=200)
		multiple_passes_settings_frame.grid(row=2, column=0)
	else:
		multiple_passes_settings_frame.grid_forget()
		printing_parameters_frame.configure(height=120)

# When clicking on any of the images it zooms in the contour (svg type) image
def zoomImage(imageName):
	
	im_ori_show_label.grid_forget()
	im_gray_show_label.grid_forget()
	im_blurred_show_label.grid_forget()
	im_contour_show_label.grid_forget()

	im_zoom_label.grid(row=0, column = 0, sticky=W)

# When clicking on the zoomed image, this zooms it out
def closeZoomImage():
	
	im_zoom_label.grid_forget()
	im_ori_show_label.grid(row=0, column = 0, sticky=W)
	im_gray_show_label.grid(row=0, column = 1, sticky=W)
	im_blurred_show_label.grid(row=1, column =0, sticky=W)
	im_contour_show_label.grid(row=1, column = 1, sticky=W)

# Starts the SVG creating process
def getSVG():
	success = IP.createSVG(image_name.get())
	if success:
		MB.showinfo(title="Success", message="SVG has been created and saved in the 'output' directory")
	else:
		MB.showerror(title="Error", message="An error occured during SVG creation")	



# The programs start with reading in the saved printer settings	
readPrinterSettings()


#---------------------------- Image upload tab ------------------------------

# Left column on the tab, holds the images

image_frame = Frame(tab1, width=660, height=900, relief="flat")
image_frame.grid(row=0, column=0)
image_frame.rowconfigure(0, weight=1)
image_frame.columnconfigure(0, weight=1)
image_frame.grid_propagate(0)

# This is kinda the same as the previous one
image_preview_frame = Frame(image_frame, width=660, height=900, relief="flat")
image_preview_frame.grid(row=0, column=0)


	# Original image
im_ori_show_label = Label(image_preview_frame, image = None, pady = 20)
im_ori_show_label.grid(row=0, column = 0, sticky=W)
im_ori_show_label.bind("<Button-1>", lambda e: zoomImage("originalImage"))

	# Gray Image 
im_gray_show_label = Label(image_preview_frame, image = None, pady = 20)
im_gray_show_label.grid(row=0, column = 1, sticky=W)
im_gray_show_label.bind("<Button-1>", lambda e: zoomImage("grayImage"))

	# Blurred Image
im_blurred_show_label = Label(image_preview_frame, image = None, pady = 20)
im_blurred_show_label.grid(row=1, column =0, sticky=W)
im_blurred_show_label.bind("<Button-1>", lambda e: zoomImage("blurredImage"))

	# Contour image
im_contour_show_label = Label(image_preview_frame, image = None, pady = 20)
im_contour_show_label.grid(row=1, column = 1, sticky=W)
im_contour_show_label.bind("<Button-1>", lambda e: zoomImage("contourImage"))

	# Zoomed image, it's hidden by default
im_zoom_label = Label(image_frame, image = tkImageContourZoom, pady = 20, width=660, height=800)
im_zoom_label.bind("<Button-1>", lambda e: closeZoomImage())
	

	# Image upload button, it hides after an image is uploaded
im_upload_button = Button(tab1, text="Upload image", command=uploadImage)
im_upload_button.place(relx=0.5, rely=0.5,anchor=CENTER)





# Contour detection parameter sliders and checkboxes (second column on the tab)


	# The frame holding all the paramaters
all_contour_settings_frame = Frame(tab1, width=340, height=800)
all_contour_settings_frame.grid_propagate(0)
all_contour_settings_frame.columnconfigure(0, weight=1)


# --------------- New image and refresh button frame ------------------------------
newimage_frame = Frame(all_contour_settings_frame, width=320, height=40, highlightbackground="black", highlightthickness=1, pady=5)
newimage_frame.grid(column=0, row=0)
newimage_frame.grid_propagate(0)

new_image_button = Button(newimage_frame,text="New image", command=uploadImage)
new_image_button.grid(column=0, row=0, padx=40)

recontour_button = Button(newimage_frame,text="Recontour image", command=setImages)
recontour_button.grid(column=1, row=0,padx=40)


# ----------- Spacing Frame ----------

spacing_frame = Frame(all_contour_settings_frame, width=320, height=20)
spacing_frame.grid(column=0, row=1)

#------------  Detection settings frame ------------------
detection_settings_frame = Frame(all_contour_settings_frame, width=320, height=370, highlightthickness=1, highlightbackground="black")
detection_settings_frame.grid(row=2, column=0)
detection_settings_frame.grid_propagate(0)
detection_settings_frame.columnconfigure(0, weight=1)

 	# Title of the group
detection_settings_label = Label(detection_settings_frame, text="Contour detection parameters")
detection_settings_label.grid(row=0, column=0, pady=10)
	
	# Detection minimum frame # row 0,
thresh_min_frame = Frame(detection_settings_frame, width=340, height=70)
thresh_min_frame.grid(row=1, column=0,padx=0, pady=10)
thresh_min_frame.grid_propagate(0)
thresh_min_frame.columnconfigure(0, weight=1)

thresh_min_label = Label(thresh_min_frame, text="Threshold minimum value")
	
thres_min_scale = Scale(thresh_min_frame, from_=1, to=254, orient=HORIZONTAL,length=300, variable=thresh_min, label="Detection minimum")
thres_min_scale.grid(row=1, column=0,padx=5, pady=0)

	#Detection maximum frame # row 1
thresh_max_frame = Frame(detection_settings_frame, width=340, height=70)
thresh_max_frame.grid(row=2, column=0, padx=0, pady=10)
thresh_max_frame.columnconfigure(0, weight=1)

thresh_max_label = Label(thresh_max_frame, text="Threshold maximum")

thres_max_scale = Scale(thresh_max_frame, from_=1, to=254, orient=HORIZONTAL,length=300, variable=thresh_max, label="Detection maximum")
thres_max_scale.grid(row=1, column=0,padx=5)

	# Inverse contour checkkox # row 2
inv_contour_checkbox = Checkbutton(detection_settings_frame, text="Inverse binary contours detection", variable=inv_contour, onvalue=True, offvalue=False)
inv_contour_checkbox.grid(row=3, column=0, pady=10)

	# Blur mode dropdrown # row 3
blur_mode_frame = Frame(detection_settings_frame, width=340, height=50)
blur_mode_frame.grid(row=4, column=0, padx=0, pady=10)
blur_mode_frame.columnconfigure(0, weight=1)

blur_mode_label = Label(blur_mode_frame, text="Blur type")
blur_mode_label.grid(row=0, column=0,padx=30)

blur_mode_dropdown = OptionMenu(blur_mode_frame, choosen_blur, *choices)
blur_mode_dropdown.grid(row=1, column=0, pady=0)

	# Image mirror frame
im_mirror_frame = Frame(detection_settings_frame, width=340, height = 50)
im_mirror_frame.grid(row=5, column=0, pady=10)

x_mirror_checkbutton = Checkbutton(im_mirror_frame, text="Horizontal mirror", variable=horizontal_mirror)
x_mirror_checkbutton.grid(row=0, column=1)

y_mirror_checkbutton = Checkbutton(im_mirror_frame, text="Vertical mirror", variable=vertical_mirror)
y_mirror_checkbutton.grid(row=0, column=2)


#----------- Spacing frame --------------

spacing_frame_2 = Frame(all_contour_settings_frame, width=320, height=20)
spacing_frame_2.grid(column=0, row=3)

#----------------------------------------



#----------- Contour area filters frame ----------------

area_thresh_frame = Frame(all_contour_settings_frame, width=320, height=40, highlightbackground="black", highlightthickness=1, pady=5, padx=5)
area_thresh_frame.columnconfigure(0, weight=1)
area_thresh_frame.grid_propagate(0)

	# Contour area filter ON/OFF chechbox
area_thresh_checkbutton = Checkbutton(area_thresh_frame, text="Contour area filter", offvalue=False, onvalue=True, variable=conture_threshold, command=hideContourThreshold)
area_thresh_checkbutton.grid(row=0, column=0,padx=5)

	# Contour area filter Maximum slider
area_thresh_min_scale = Scale(area_thresh_frame, from_=0, to=1, orient=HORIZONTAL,length=300, variable=min_contour_area, label="Min area", showvalue=False)
area_thresh_min_scale.bind("<Right>", lambda e: area_thresh_min_scale.set(area_thresh_min_scale.get() + 9 ))
area_thresh_min_scale.bind("<Left>", lambda e: area_thresh_min_scale.set(area_thresh_min_scale.get() - 9 ))
	
	# Contour area filter Minimum slider
area_thresh_max_scale = Scale(area_thresh_frame, from_=0, to=1, orient=HORIZONTAL,length=300, variable=max_contour_area, label="Max area", showvalue=False)
area_thresh_max_scale.bind("<Right>", lambda e: area_thresh_max_scale.set(area_thresh_max_scale.get() + 9 ))
area_thresh_max_scale.bind("<Left>", lambda e: area_thresh_max_scale.set(area_thresh_max_scale.get() - 9 ))

#----------- Spacing frame --------------

spacing_frame_2 = Frame(all_contour_settings_frame, width=320, height=20)
spacing_frame_2.grid(column=0, row=5)

#----------------------------------------

# --------------- Create SVG and Recontour button frame ------------------------------
separate_window_frame = Frame(all_contour_settings_frame, width=320, height=40, highlightbackground="black", highlightthickness=1, pady=5)
separate_window_frame.grid(column=0, row=6)
separate_window_frame.grid_propagate(0)
separate_window_frame.columnconfigure(0, weight=0)
separate_window_frame.rowconfigure(0, weight=1)

create_SVG_button = Button(separate_window_frame,text="Generate SVG", command=getSVG)
create_SVG_button.grid(column=0, row=0, padx=40)

recontour_button_2 = Button(separate_window_frame,text="Recontour image", command=setImages)
recontour_button_2.grid(column=1, row=0,padx=40)




#----Close button # don't really think is neccessary------------

close_button = Button(tab1, text="Close", command=root.destroy)
#close_button.grid(row=3, column=3)





#----------------- G-code generate tab --------------


# Printing settings frame

printing_settings_frame = Frame(tab2, width=450, height=700, padx=20, pady=20, highlightthickness=0, highlightbackground="black")
printing_settings_frame.grid(column=0, row=0)
printing_settings_frame.grid_propagate(0)
printing_settings_frame.columnconfigure(0, weight=1)
printing_settings_frame.rowconfigure(0, weight=0)

# Printing area settings frame

printing_area_frame = Frame(printing_settings_frame, width=350, height=400, highlightbackground="black", highlightthickness=1)
printing_area_frame.grid(column=0, row=0, padx=10, pady=10)
printing_area_frame.grid_propagate(0)
printing_area_frame.columnconfigure(0, weight=0)
printing_area_frame.rowconfigure(0, weight=0)

#	 Work height label + entry
work_height_label = Label(printing_area_frame, text="Work height coord. (mm): ")
work_height_label.grid(row=0, column=0, pady=15)

work_height_entry = Entry(printing_area_frame, textvariable=work_height)
work_height_entry.grid(row=0, column=1, pady=15)

	# Lift height label + entry
lift_height_label = Label(printing_area_frame, text="Move height  coord. (mm): ")
lift_height_label.grid(row=1, column=0, pady=15)

lift_height_entry = Entry(printing_area_frame, textvariable=lift_height)
lift_height_entry.grid(row=1, column=1, pady=15)

	# Safe height label + entry
safe_height_label = Label(printing_area_frame, text="Safe height coord. (mm): ")
safe_height_label.grid(row=2, column=0, pady=15)

safe_height_entry = Entry(printing_area_frame, textvariable=safe_height)
safe_height_entry.grid(row=2, column=1, pady=15)
	
	#Left X label + entry
left_x_label = Label(printing_area_frame, text="Bottom Left X coord. (mm): ")
left_x_label.grid(row=3, column=0, pady=15)

left_x_entry = Entry(printing_area_frame, textvariable=left_x)
left_x_entry.grid(row=3, column=1, pady=15)

	#Left Y label + entry
left_y_label = Label(printing_area_frame, text="Bottom Left Y coord. (mm): ")
left_y_label.grid(row=4, column=0, pady=15)

left_y_entry = Entry(printing_area_frame, textvariable=left_y)
left_y_entry.grid(row=4, column=1, pady=15)

	#Right X label + entry
right_x_label = Label(printing_area_frame, text="Top Right X coord. (mm): ")
right_x_label.grid(row=5, column=0, pady=15)

right_x_entry = Entry(printing_area_frame, textvariable=right_x)
right_x_entry.grid(row=5, column=1, pady=15)

	#Right Y label + entry
right_y_label = Label(printing_area_frame, text="Top Right Y coord. (mm): ")
right_y_label.grid(row=6, column=0, pady=15)

right_y_entry = Entry(printing_area_frame, textvariable=right_y)
right_y_entry.grid(row=6, column=1, pady=15, padx=50)

	# Save settings button
save_options_button = Button(printing_area_frame, text="Save options", command=savePrinterSettings)
save_options_button.grid(row=7, column=0,columnspan=2)


# ----------- Spacing Frame ----------

spacing_frame = Frame(printing_settings_frame, width=320, height=30)
spacing_frame.grid(column=0, row=1, padx=10, pady=0)

#--------------------------------------


# --------- Other printing parameters
	
printing_parameters_frame = Frame(printing_settings_frame, width=350, height=120, highlightbackground="black", highlightthickness=1)
printing_parameters_frame.grid(column=0, row=2)
printing_parameters_frame.grid_propagate(0)
printing_parameters_frame.columnconfigure(0, weight=1)
printing_parameters_frame.rowconfigure(0, weight=0)

	# Printing speed slider
printing_speed_scale = Scale(printing_parameters_frame, from_=100, to=2500, orient=HORIZONTAL,length=300, variable=printing_speed, label="Printing speed", showvalue=True)
printing_speed_scale.grid(row=0, column=0, columnspan=2, pady=10)

	# Multiple passes/layers ON/OFF checkbox
multiple_passes_checkbutton = Checkbutton(printing_parameters_frame, variable=isMultiplePasses, text="Multiple passes, layers", onvalue=True, offvalue=False, command=hideMultiplePasses)
multiple_passes_checkbutton.grid(row=1, column=0, pady=5)

	# Multiple passes/layers settings, on visible when corresponding checkbox in ON
multiple_passes_settings_frame = Frame(printing_parameters_frame)

	# Number of passes/layers label + entry
multiple_passes_num_label = Label(multiple_passes_settings_frame, text="Number of passes: ")
multiple_passes_num_label.grid(row=0, column=0, pady=5)

multiple_passes_num = Entry(multiple_passes_settings_frame, textvariable=multiplePassesNum)
multiple_passes_num.grid(row=0, column=1, pady=5, padx=50)

	# Passes/layers height decrease label + entry
multiple_passes_height_label = Label(multiple_passes_settings_frame, text="Work height decrease\n between passes (mm): ")
multiple_passes_height_label.grid(row=1, column=0, pady=5)

multiple_passes_height_entry = Entry(multiple_passes_settings_frame, textvariable=multiplePassesNumHeight)
multiple_passes_height_entry.grid(row=1, column=1, pady=5, padx=50)


# -------------- GCODE output frame -----------------

gcode_padding_area_frame = Frame(tab2, width=500, height=700, highlightbackground="black", highlightthickness=0, pady=0, padx=20)
gcode_padding_area_frame.grid(column=1, row=0)
gcode_padding_area_frame.grid_propagate(0)
gcode_padding_area_frame.columnconfigure(0, weight=1)
gcode_padding_area_frame.rowconfigure(0, weight=0)

gcode_area_frame = Frame(gcode_padding_area_frame, highlightthickness=1, highlightbackground="black", width=460, height=560)
gcode_area_frame.grid(row=0, column=1, pady=20, padx=10)
gcode_area_frame.grid_propagate(0)
gcode_area_frame.columnconfigure(0, weight=1)
gcode_area_frame.rowconfigure(0, weight=0)


	# G code output text area
gcode_text_area = scrolledtext.ScrolledText(gcode_area_frame, width=50)
gcode_text_area.grid(row=0,column=0, rowspan=1, pady=20)

	# Create G code button
gcode_gen_button = Button(gcode_area_frame, text="Generate G-code from processed image", command=createGcode, state="active")
gcode_gen_button.grid(row=1, column=0)

	# G-code save to file button # only active when gcode has been generated already
gcode_gen_save_button = Button(gcode_area_frame, text="Save generated G-gode to file", command=saveGcodeFile, state="disabled")
gcode_gen_save_button.grid(row=2, column=0, pady=20)



#----------------------- Help tab ------------------------

help_frame = Frame(tab3, width=800, height=600, highlightbackground="black", highlightthickness=0, pady=25, padx=5)
help_frame.grid(row=0, column=0)
help_frame.columnconfigure(0, weight=1)
#help_frame.rowconfigure(0, weight=1)
help_frame.grid_propagate(0)

help_text_area = scrolledtext.ScrolledText(help_frame, height=400)

help_text_area.grid(row=0, column=0)

# Starts program
root.mainloop()





