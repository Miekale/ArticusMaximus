// Functions required to move and draw robot
void move_pen(float pos_0, float pos_1);
void calc_motor_power(float angle, int max_power, int* motor_powers);
float calc_angle(float pos_0, float pos_1);

// ---- UNIT TESTS -----
task main()
{
	return;
}

// Function Definitions
void move_pen(float* pos_0, float* pos_1, bool draw, int max_draw_power, int max_move_power, int pen_distance)
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

	// Get angle
	float angle = calc_angle(pos_0, pos_1); 
	
	// Get motor power
	float motor_powers[2] = {0,0};
	calc_motor_power(angle, max_move_power, motor_powers); 
	
	// If draw then lower pen
	if (draw)
	{
		pen_down();
	}

	// Move motors at power
	motor[motorA] = motor_powers[0]; 
	motor[motorD] = motor_powers[1];

	// Keep moving motor until either x or y target passed
	// 1st Quadrant 
	if (angle < 90)
	{
		while (nMotorEncoder[motorA] < pos_1[0] || nMotorEncoder[motorA] < pos_1[1]) 
		{}
	}
	// 2nd Quadrant
	else if (angle < 180)
	{
		while (nMotorEncoder[motorA] > pos_1[0] || nMotorEncoder[motorA] < pos_1[1]) 
		{}
	}
	// 3rd Quadrant
	else if (angle < 270)
	{
		while (nMotorEncoder[motorA] > pos_1[0] || nMotorEncoder[motorA] > pos_1[1]) 
		{}
	}
	// 4th Quadrant
	else
	{
		while (nMotorEncoder[motorA] < pos_1[0] || nMotorEncoder[motorA] > pos_1[1]) 
		{}
	}
	motor[motorA] = motor[motorD] = 0; 
	
	if (draw)
	{
		pen_up();
	}
	
	return;

}


void calc_motor_power(float angle, int max_power, int* motor_powers)
{
	/*
	Calculates x motor and y motor power from starting and ending positions,
	scaled by max power. Returns by reference

	PARAMETERS
	----------
	pos_0: float array[x,y], starting position.
	pos_1: float array[x,y], target position.
	max_power: int, positive < 100, maximum power allowed.
	motor_powers: int array[power x, power y], reference output.

	RETURNS
	-------
	motor_powers: int array[power x, power y]

	*/
	motor_powers[0] = max_power * cos(angle); 
	motor_powers[1] = max_power * sin(angle); 

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
	float delta_x = pos_1[0] - pos_0[0];
	float delta_y = pos_1[1] - pos_0[1];
	float angle = rad_to_deg(atan(delta_y / delta_x));
	
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
	else if (delta_y > 0 && delta_x < 0)
	{
		angle += 360; 
	}
	
	// 1st Quadrant do nothing
	return angle; 
}
