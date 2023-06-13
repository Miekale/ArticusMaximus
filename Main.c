// Main Pseudocode

void initialize_sensors()
{
	//touch x
	SensorType[S1] = sensorEV3_Touch;
	//touch y
	SensorType[S2] = sensorEV3_Touch;

	SensorType[S3] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[S3] = modeEV3Color_Color;
	wait1Msec(50);

	SensorType[S4] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorMode[S4] = modeEV3Gyro_Calibration;
	wait1Msec(100);
	SensorMode[S4] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

// motor A = x movement
// motor B = y movement
// motor D = z movement


task main()
{
	const float x_axis_gear_radius = 0;
	const float y_axis_gear_radius = 0;
	const float z_axis_gear_radius = 0;
	const float pen_distance = 5;

	float x = ;
	float y = ;

	// Initialize Sensors
	initialize_sensors();
	nMotorEncoder[motorD] = 0;
	// Open File

	// Zero axises

	// ---- DRAWING LOOP ---- //
	/* while file has instructions
		if draw function:
			draw
		else:
			move

	*/

	// close file
}
