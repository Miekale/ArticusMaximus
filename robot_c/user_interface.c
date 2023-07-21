const int MAX_NUM_FILES = 10;

bool pointer_position(int & pointer)
{
	while(!getButtonPress(buttonAny))
	{}
	if(getButtonPress(buttonDown))
	 {
		if(pointer < 3)
				pointer++;
		else
				pointer = 1; //reset pointer back to 1 if in third option
	 }
	 else if(getButtonPress(buttonUp))
	   {
			if(pointer > 1)
			{
				pointer--;
			}
			else
				pointer = 3; //reset pointer back to 1 if in third option
		 }
		else if(getButtonPress(buttonEnter) && pointer == 3)
		{
			pointer = 0;
		}
		else if(getButtonPress(buttonEnter) && pointer!= 3)
			return true;

		return false;
}

void main_disp(int pointer)
{
	displayString(3, "-----ARTICUS MAXIMUS-----");
	displayString(6, "Currently selected: %d", pointer);
	displayString(8, "1) Select a drawing file");
	displayString(9, "2) Draw a basic shape");
	displayString(10, "3) Exit menu");
}

//create a function for each display

void dispFiles(int pointer)
{
	displayString(3, "Please select a file!");
	displayString(6, "Currently selected: %d", pointer);
	displayString(8, "1) BLAH BLAH BLAH");
	displayString(9, "2) BLAH BLAH");
	displayString(10, "3) Go to menu");

}

void dispShapes(int pointer)
{
	displayString(3, "Please select a basic shape: ");
	displayString(6, "Currently selected: %d", pointer);
	displayString(8, "1) BLAH BLAH BLAH");
	displayString(9, "2) BLAH BLAH");
	displayString(10, "3) Go to menu");
}

// void basic shapes
// void exit function

task main()
{
	// list of files

	// pointer to select the sindows option
	int pointer = 1;
	while(pointer != 0)
	{
		bool sub_menu = false;
		main_disp(pointer);
		sub_menu = pointer_position(pointer);
		if (sub_menu)
		{
			  int pointer2 = 1;
			  eraseDisplay();
			  wait1Msec(500);
			  bool run_code = false;
				while(pointer2 != 0)
				{
					if(pointer == 1)
						dispFiles(pointer2);
					else
						dispShapes(pointer2);
					run_code = pointer_position(pointer2);
					wait1Msec(500);
				}
				eraseDisplay();
				wait1Msec(500);
		}

		wait1Msec(300);
	}
}
