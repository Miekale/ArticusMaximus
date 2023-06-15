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
void move_pen(float* pos_0, float* pos_1, bool draw=False, int max_draw_power, int max_move_power)
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

	// Determine angle

	// Determine motor power

	// Move motors at power

	// Keep moving motor until limit reached / passed

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

	// Funny Trig

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
	
	// 1st or 2nd Quadrant
	if (delta_y >0)
	{
		angle += 180; 
	}
	
	// 3rd of 4th Quadrant
	
	return angle; 
}
