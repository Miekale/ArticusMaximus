
float degrees_to_mm (int degrees, float wheel_radius)
{
	return degrees * wheel_radius * PI / 180;
}

float mm_to_degrees (float distance, float wheel_radius)
{
	return distance * 180 / wheel_radius / PI;
}

task main()
{



}
