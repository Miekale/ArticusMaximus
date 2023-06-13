void pen_up(float pen_distance)
{
	// Setting motor to run forwards until distance is pen distance away from the page
	motor[motorD] = 25;
	while(pen_distance > degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
	{}
	motor[motorD] = 0;
}

void pen_down(float pen_distance)
{
	// Setting motor runs backwards until distance is backwards to 0mm
	motor[motorD] = -25;
	while(0 > degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
	{}
	motor[motorD] = 0;
}

task main()
{
	nMotorEncoder[motorD] = 0;
	pen_down();
	pen_up();
}
