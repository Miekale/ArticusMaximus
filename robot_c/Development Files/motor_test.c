#include "PC_FileIO.c"

task main()
{
	TFileHandle fout;
	openWritePC(fout, "output.txt");

	for (int motor_power = 0; motor_power <= 100; motor_power++)
	{
		motor[motorA] = motor_power;
		wait1Msec(1000);
		time1[T1] = 0;
		nMotorEncoder[motorA] = 0;
		while (time1[T1] < 5000)
		{}
		writeFloatPC(fout, "%.2f",nMotorEncoder[motorA] * 1.0);
		writeEndlPC(fout);
	}
	writeEndlPC(fout);
	wait1Msec(1000);
	closeFilePC(fout);
}
