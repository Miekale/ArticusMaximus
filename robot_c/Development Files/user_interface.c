const int MAX_FILES = 4;
const int MAX_SHAPES = 2;

const string fileArray[MAX_FILES] = {"File 1", "File 2", "File 3", "File 4"};
const string shapeArray[MAX_SHAPES] = {"Square", "Circle"};

const string fileNames[MAX_FILES] = {"file1.txt", "file2.txt", "file3.txt", "file4.txt"};
const string shapeNames[MAX_SHAPES] = {"shape1.txt", "shape2.txt"};

bool movePointer(int &pointer, int options);
void dispMain(int pointer);
void dispShapes(int pointer);
void dispFiles(int pointer);

bool movePointer(int &pointer, int options)
{
	while (!getButtonPress(buttonAny))
	{}
	if (getButtonPress(buttonDown))
	{
		if (pointer < options)
				pointer++;
		else
				pointer = 1; //look back to the first option
	}
	else if (getButtonPress(buttonUp)) //up button is pressed
	{
			if (pointer > 1)
				pointer--;
			else
				pointer = options; //loop back to the last option
	}
	else if (getButtonPress(buttonEnter) && pointer == options)//last option is for exit
		pointer = 0;

	else if (getButtonPress(buttonEnter) && pointer!= options)
		return true; //return true to go into a sub menu based on pointer's position

	return false; //return false to continue moving the pointer
}

void dispMain(int pointer)
{
	displayString(3, "-----ARTICUS MAXIMUS-----");
	displayString(5, "1) Draw from a file");
	displayString(6, "2) Draw a basic shape");
	displayString(7, "3) Exit menu");

	displayString(10, "Currently selected: %d", pointer);
}

//create a function for each display
void dispFiles(int pointer)
{
	displayString(3, "Please select a file: ");

  for (int index = 0; index < MAX_FILES; index++)
  	displayString(index + 5, "%d) %s", index + 1, fileArray[index]);

  displayString(MAX_FILES + 5, "%d) Go back to main menu", MAX_FILES + 1);
  displayString(12, "Currently selected: %d", pointer);
}

void dispShapes(int pointer)
{
    displayString(3, "Please select a basic shape: ");

    for (int index = 0; index < MAX_SHAPES; index++)
    	displayString(index + 5, "%d) %s", index + 1, shapeArray[index]);

    displayString(MAX_SHAPES + 5, "%d) Go back to main menu", MAX_SHAPES + 1);
    displayString(12, "Currently selected: %d", pointer);
}

// void basic shapes
// void exit function

task main()
{
    const int HOLDTIME = 300; // delay time in milleseconds

	// pointer to select the windows option
	int pointer = 1;
	int main_option = 3;

	while (pointer != 0)
	{
		bool sub_menu = false; //condition to load any sub menu
		dispMain(pointer);
		sub_menu = movePointer(pointer, main_option);
		if (sub_menu)
		{
			  int sub_pointer = 1;
			  eraseDisplay();
			  wait1Msec(HOLDTIME);
			  bool run_code = false;
				while(sub_pointer != 0)
				{
					int sub_options = 0;

					if(pointer == 1)
					{
						sub_options = MAX_FILES + 1;
						dispFiles(sub_pointer);
					}

					else
					{
						sub_options = MAX_SHAPES + 1;
						dispShapes(sub_pointer);
					}

					run_code = movePointer(sub_pointer, sub_options);

					if(run_code)
					{
						eraseDisplay();
						wait1Msec(HOLDTIME);

						if(pointer == 1)
						{
							displayString(5, "%s", fileNames[sub_pointer - 1]);
							displayString(7, "Press enter to return back");
							while(!getButtonPress(buttonEnter))
							{}
						}

						else
						{
							displayString(5, "%s", shapeNames[sub_pointer - 1]);
							displayString(7, "Press enter to return back");
							while(!getButtonPress(buttonEnter))
							{}
						}
						eraseDisplay();
					}
				  wait1Msec(HOLDTIME);
				} //while ends
				eraseDisplay();
				wait1Msec(HOLDTIME);
		}
		wait1Msec(HOLDTIME);
	}
}
