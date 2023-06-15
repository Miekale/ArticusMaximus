void initilizeSensors()
{
	SensorType[S1] = sensorEV3_Touch;
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

void zero(float* pos)
{
	//zeroing speeds
	int speed_initial = 25;
	int speed_final = 10;

	//testing if on the sensor intitially and moving off the sensor
	if (SensorValue[S1] == 1)
	{
		motor[motorA] = -speed_initial;
		wait1Msec(1000);
		motor[motorA] = 0;
	}

	if (SensorValue[S2] == 1)
	{
		motor[motorB] = -speed_initial;
		wait1Msec(1000);
		motor[motorB] = 0;
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
	motor[motorB] = speed_initial;
	while (!SensorValue[S2])
	{}
	motor[motorB] = 0;
	wait1Msec(50);
	motor[motorB] = -speed_final;
	while (SensorValue[S2])
	{}
	motor[motorB] = 0;
	motor[motorB] = speed_final;
	while (!SensorValue[S2])
	{}
	motor[motorB] = 0;
	//initilize quardinate 0, 0
	pos[0] = 0;
	pos[1] = 0;
}


task main()
{



}
