// Conversion functions

// Converts degrees gear turned to linear distance (mm)
float degrees_to_mm (int degrees, float gear_radius)
{
	return degrees * wheel_radius * PI / 180;
}

// Converts linear distance (mm) to degrees for gear turning
float mm_to_degrees (float distance, float gear_radius)
{
	return distance * 180 / wheel_radius / PI;
}

// Degrees to radians
float deg_to_rad(deg)
{
	return deg*PI/180.0; 
}

// Radians to degrees 
float rad_to_deg(rad)
{
	return rad/PI*180.0;
}

task main()
{



}
