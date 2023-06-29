const float PEN_HEIGHT = 5;

void pen_up();
void pen_down(); 

void pen_down()
{
	// Setting motor to run forwards until distance is pen distance away from the page
	motor[motorD] = 25;
	while(PEN_HEIGHT > degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
	{}
	motor[motorD] = 0;
}

void pen_up()
{
	// Setting motor runs backwards until distance is backwards to 0mm
	motor[motorD] = -25;
	while(0 < degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
	{}
	motor[motorD] = 0;
}

task main()
{
	nMotorEncoder[motorD] = 0;
	pen_down();
	pen_up();
}
