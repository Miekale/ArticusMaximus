#include "PC_FileIO.c"

// DEBUG OUTPUT
TFileHandle fout;

// Global Hardware Constants
float const GEAR_RADIUS_X = 11.93; // mm
float const GEAR_RADIUS_Y = 11.93;
float const GEAR_RADIUS_Z = 13.081;
float const PEN_DISTANCE = 1;

// Global Software Constants (UI and File information)
const int MAX_FILES = 4;
const int MAX_SHAPES = 2;

const string fileArray[MAX_FILES] = {"Among Us", "NASSAR GOAT", "Custom File 3", "Custom File 4"};
const string shapeArray[MAX_SHAPES] = {"Square", "Circle"};

const string fileNames[MAX_FILES] = {"amongus.txt", "goat.txt", "contour_output_3.txt", "contour_output_4.txt"};
const string shapeNames[MAX_SHAPES] = {"square.txt", "circle.txt"};

// ------ FUNCTION PROTOTYPES ------ //

//UI FUNCTIONS
bool movePointer(int &pointer, int options);
void dispMain(int pointer);
void dispShapes(int pointer);
void dispFiles(int pointer);

// CONVERSION FUNCTIONS
float degrees_to_mm (int degrees, float gear_radius);
float mm_to_degrees (float distance, float gear_radius);
float deg_to_rad(float deg);
float rad_to_deg(float rad);
void pos_mm_to_degree(float* mm_pos, float* deg_pos);
void pos_degree_to_mm(float* mm_pos, float* deg_pos);

// ROBOTC MOVEMENT FUNCTIONS
void initialize_sensors();
void get_current_pos(float* mm_pos);
void zero();
void move_pen_z(bool move_up);
float calc_angle(float* pos_0, float* pos_1);
void calc_motor_power(float angle, int max_power, float* motor_powers);
void draw_or_move(float* target_pos, bool draw, int max_draw_power, int max_move_power);
void draw_image_from_file(string file_name);

// ------ FUNCTION DEFINITIONS ------ //
bool movePointer(int &pointer, int options)
{
	while (!getButtonPress(buttonAny))
	{}
	if (getButtonPress(buttonDown))
	{
		if (pointer < options)
			pointer++;
		//loop back to the first option
		else
			pointer = 1;
	}
	//move the currently selected option up
	else if (getButtonPress(buttonUp))
	{
		if (pointer > 1)
			pointer--;
	//loop back to the last option
		else
			pointer = options;
	}
	//set pointer to 0 if user presses enter to exit menu
	else if (getButtonPress(buttonEnter) && pointer == options)
		pointer = 0;
	//return true to go into the desired sub menu
	else if (getButtonPress(buttonEnter) && pointer!= options)
		return true;

	//return false to continue moving the pointer
	return false;
}

void dispMain(int pointer)
{
	displayString(3, "-----ARTICUS MAXIMUS-----");
	displayString(5, "1) Draw from a file");
	displayString(6, "2) Draw a basic shape");
	displayString(7, "3) Exit menu");

	displayString(10, "Currently selected: %d", pointer);
}

//create a
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
// Degrees gear turned to linear distance (mm)
float degrees_to_mm (int degrees, float gear_radius)
{
	return degrees * gear_radius * PI / 180;
}
// Linear distance (mm) to degrees for gear turning
float mm_to_degrees (float distance, float gear_radius)
{
	return distance * 180 / gear_radius / PI;
}
// Degrees to radians
float deg_to_rad(float deg)
{
	return deg*PI/180.0;
}
// Radians to degrees
float rad_to_deg(float rad)
{
	return rad/PI*180.0;
}
// Converts pos(x,y) from mm to degrees, returns by reference
void pos_mm_to_degree(float* mm_pos, float* deg_pos)
{
	deg_pos[0] = mm_to_degrees(mm_pos[0], GEAR_RADIUS_X);
	deg_pos[1] = mm_to_degrees(mm_pos[1], GEAR_RADIUS_Y);
}

void pos_degree_to_mm(float* mm_pos, float* deg_pos) {
	mm_pos[0] = degrees_to_mm(deg_pos[0], GEAR_RADIUS_X);
	mm_pos[1] = degrees_to_mm(deg_pos[1], GEAR_RADIUS_Y);
}

// Initialize Touch Sensors
void initialize_sensors()
{
	//touch x
	SensorType[S1] = sensorEV3_Touch;
	//touch y
	SensorType[S2] = sensorEV3_Touch;
	SensorType[S3] = sensorEV3_Color;

}
// Get current pen position in mm
void get_current_pos(float* mm_pos)
{
	float deg_pos[2] = {0, 0};
	deg_pos[0] = -nMotorEncoder[motorA];
	deg_pos[1] = -nMotorEncoder[motorD];
	pos_degree_to_mm(mm_pos, deg_pos);
}
// Zero pen x, y, z
void zero()
{
	int const speed_initial = 20;
	int const speed_final = 10;
	//testing if on the sensor initially and moving off the sensor
	if (SensorValue[S1] == 1)
	{
		motor[motorA] = -speed_initial;
		wait1Msec(250);
		motor[motorA] = 0;
	}

	if (SensorValue[S2] == 1)
	{
		motor[motorD] = -speed_initial;
		wait1Msec(250);
		motor[motorD] = 0;
	}
	//move x and y until they hit the touch sensor
	//x movement
	motor[motorA] = speed_initial;
	while (!SensorValue[S1])
	{}
	motor[motorA] = 0;
	wait1Msec(50);
	motor[motorA] = -speed_final;
	while (SensorValue[S1])
	{}
	motor[motorA] = 0;
	motor[motorA] = speed_final;
	while (!SensorValue[S1])
	{}
	motor[motorA] = 0;
	//y movement
	motor[motorD] = speed_initial;
	while (!SensorValue[S2])
	{}
	motor[motorD] = 0;
	wait1Msec(50);
	motor[motorD] = -speed_final;
	while (SensorValue[S2])
	{}
	motor[motorD] = 0;
	motor[motorD] = speed_final;
	while (!SensorValue[S2])
	{}
	motor[motorD] = 0;

	// Set motor encoder positioning
	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorD] = 0;
}
// Moves pen up or down to page
void move_pen_z(bool move_up)
{
	if (move_up)
	{
		// Setting motor to run forwards until distance is pen distance away from the page
		motor[motorB] = -25;
		//wait1MSec(100);
		while(0 < nMotorEncoder[motorB])
		{}
		motor[motorB] = 0;
	}
	else
	{
		// Setting motor runs backwards until distance is backwards to 0mm
		motor[motorB] = 25;
		while(40 > nMotorEncoder[motorB])
		{}
		motor[motorB] = 0;
	}
}

void calc_motor_power(float angle, int max_power, int* motor_powers)
{
	/*
	Calculates x motor and y motor power from input angle,
	scaled by max power. Returns by reference

	PARAMETERS
	----------
	angle: float, radians, horizontal from [1,0] of between start and end points
	max_power: int, positive < 100, maximum power allowed.
	motor_powers: int array[power x, power y], reference output.

	RETURNS
	-------
	motor_powers: int array[power x, power y]

	*/
	angle = deg_to_rad(angle);
	motor_powers[0] = round(max_power * cos(angle));
	motor_powers[1] = round(max_power * sin(angle));
	return;
}

float calc_angle(float* pos_0, float* pos_1)
{
	/*
	Calculates angle (degrees) between two points,
	relative to [1, 0] vector.

	PARAMETERS
	----------
	pos_0: float array[x,y], starting position.
	pos_1: float array[x,y], target position.

	RETURNS
	-------
	angle: float degrees between the two points
	*/

	float delta_x = pos_1[0] - pos_0[0]; // 30 - 0 = 30
	float delta_y = pos_1[1] - pos_0[1]; // 0 - 30 = -30
	float angle = 0;
	// Debug
	writeFloatPC(fout, "%f ", pos_0[0]);
	writeFloatPC(fout, "%f starting point", pos_0[1]);
	writeEndlPC(fout);
	writeFloatPC(fout, "%f ", pos_1[0]);
	writeFloatPC(fout, "%f ending point", pos_1[1]);
	writeEndlPC(fout);
	// Vertical lines
	if (delta_x == 0)
	{
		if (delta_y < 0)
		{
			angle = 270;
		}
		else
		{
			angle = 90;
		}
		// Debug
		writeFloatPC(fout, "returned angle: %.2f", angle);
		return angle;
	}
	// Non vertical lines
	angle = rad_to_deg(atan(delta_y / delta_x));
	// 2nd Quadrant
	if (delta_y > 0 && delta_x < 0)
	{
		angle += 180;
	}
	// 3rd Quadrant
	else if (delta_y < 0 && delta_x < 0)
	{
		angle += 180;
	}
	// 4th Quadrant
	else if (delta_y < 0 && delta_x > 0)
	{
		angle += 360;
	}
	// 1st Quadrant do nothing
	// Debug
	writeFloatPC(fout, "returned angle: %.2f", angle);
	return angle;
}

void draw_or_move(float* target_pos, bool draw, int max_draw_power, int max_move_power)
{
	/* Controls x motor and y motor to move pen from starting position
	to ending position.

	PARAMETERS
	----------
	pos_0: float array[x,y], starting position.
	pos_1: float array[x,y], target position.
	max_draw_power: int <100, maximum motor power allowed while drawing
	max_move_power: int <100, maximum motor power allowed while moving

	RETURNS
	-------
	void
	*/
	float const POS_TOL = 1;  // pen move within 1mm of actual target

	// Initialize starting positions
	float current_pos[2] = {0, 0};
	get_current_pos(current_pos);

	// make target position negative
	float actual_target[2] = {target_pos[0] * 1, target_pos[1] * 1};

	// Drawing mode
	if (draw)
	{
		// Get motor powers
		float motor_powers[2] = {0,0};
		float angle = calc_angle(current_pos, target_pos);

		calc_motor_power(angle, max_draw_power, motor_powers);
		move_pen_z(false);

		// Debug
		//displayString(3, "%f target angle", angle);
		//displayString(6, "%d and %d is motor_powers", motor_powers[0], motor_powers[1]);
		//displayString(6, "%f %f motor powers", motor_powers[0], motor_powers[1]);
		//displayString(9, "%f %f target position", actual_target[0], actual_target[1]);
		//displayString(12, "%f %f current position", current_pos[0], current_pos[1]);
		//wait1Msec(2000);

		// Keep moving motor until end position reached
		bool x_passed_target = false;
		float x_0 = current_pos[0];
		bool y_passed_target = false;
		float y_0 = current_pos[1];

		// Move motors at power
		motor[motorA] = -motor_powers[0];
		motor[motorD] = -motor_powers[1];

		while (((abs(current_pos[0] - actual_target[0]) > POS_TOL) || (abs(current_pos[1] - actual_target[1]) > POS_TOL)) && !x_passed_target && !y_passed_target)
		{
			get_current_pos(current_pos);

			if (x_0 > actual_target[0] && actual_target[0] - current_pos[0] > POS_TOL)
			{
				x_passed_target = true;
				writeTextPC(fout, "PASSED TARGETx1");
			}
			else if (x_0 < actual_target[0] && current_pos[0] - actual_target[0] > POS_TOL)
			{
				x_passed_target = true;
				writeTextPC(fout, "PASSED TARGETx2");
			}

			if (y_0 > actual_target[1] && actual_target[1] - current_pos[1] > POS_TOL)
			{
				y_passed_target = true;
				writeTextPC(fout, "PASSED TARGETy1");
			}
			else if (y_0 < actual_target[1] && current_pos[1] - actual_target[1] > POS_TOL)
			{
				y_passed_target = true;
				writeTextPC(fout, "PASSED TARGETy2");
			}
			//displayString(15, "%f %f target position", actual_target[0], actual_target[1]);
			//displayString(17, "%f %f current position", current_pos[0], current_pos[1]);
		}

		motor[motorA] = motor[motorD] = 0;
	}
	// Moving mode
	else
	{
		// Move x motor until target
		move_pen_z(true);
		if (current_pos[0] < actual_target[0])
		{
			motor[motorA] = -max_move_power;
		}
		else if (current_pos[0] > actual_target[0])
		{
			motor[motorA] = max_move_power;
		}
		else
		{
			motor[motorA] = 0;
		}

		while ((abs(current_pos[0] - actual_target[0]) > POS_TOL))
		{
			// Debug
			//displayString(5, "%f target position", actual_target[0]);
			//displayString(7, "%f current position", current_pos[0]);
			//displayString(9, "%f DIFFERENCE", abs(current_pos[0] - actual_target[0]));
			get_current_pos(current_pos);
		}
		motor[motorA] = 0;

		// Move y motor until target
		if (current_pos[1] < actual_target[1])
		{
			motor[motorD] = -max_move_power;
		}
		else if (current_pos[1] > actual_target[1])
		{
			motor[motorD] = max_move_power;
		}
		else
		{
			motor[motorD] = 0;
		}

		while ((abs(current_pos[1] - actual_target[1]) > POS_TOL))
		{
			// Debug
			//displayString(5, "%f target position", actual_target[1]);
			//displayString(7, "%f current position", current_pos[1]);
			//displayString(9, "%f DIFFERENCE", abs(current_pos[1] - actual_target[1]));
			get_current_pos(current_pos);
		}
		motor[motorD] = 0;
		move_pen_z(false);
	}
	return;
}

void draw_image_from_file(string file_name)
{
	int const MAX_DRAW_POWER = 13;
	int const MAX_MOVE_POWER = 20;
	// Input File Validation
	TFileHandle fin;
	bool fileOkay = openReadPC(fin, file_name);
	if (!fileOkay) {
		displayString(5, "FILE READ ERROR!");
		wait1Msec(3000);
		return;
	}

	// Initialize position and zero pen
	zero();
	move_pen_z(false);

	// ---- DRAWING LOOP ---- //
	// Read line-by-line
	string move_or_draw;
	while (readTextPC(fin, move_or_draw))
	{
		// Read next point
		float next_point[2] = {0,0};
		readFloatPC(fin, next_point[0]);
		readFloatPC(fin, next_point[1]);

		// Update boolean move or draw depending on input
		bool is_draw = false;
		if (move_or_draw == "D")
		{
			is_draw = true;
		}

		// Move to point
		draw_or_move(next_point, is_draw, MAX_DRAW_POWER, MAX_MOVE_POWER);
		// Output file debug
		/*
		writeTextPC(fout, move_or_draw);
		writeTextPC(fout, " ");
		writeFloatPC(fout, next_point[0]);
		writeTextPC(fout, " ");
		writeFloatPC(fout, next_point[1]);
		writeEndlPC(fout);
		*/
	}
	// close file
	closeFilePC(fin);
	move_pen_z(true);
	zero();
}

task main()
{
	//delay time for EV3 in milleseconds
	const int HOLDTIME = 300;

	initialize_sensors();

	//initializing option pointer
	int pointer = 1;
	//number of actions in main menu
	int main_option = 3;

	while (pointer != 0)
	{
		bool sub_menu = false;
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
						string temp_file = fileNames[sub_pointer - 1];
						draw_image_from_file(temp_file);
						displayString(7, "Press enter to return back");
						while(!getButtonPress(buttonEnter))
						{}
					}

					else
					{
						string temp_file = shapeNames[sub_pointer - 1];
						draw_image_from_file(temp_file);
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
