float const PI = 3.1415;
float const GEAR_RADIUS_X = 1;
float const GEAR_RADIUS_Y = 1;

// Declarations
float degrees_to_mm (int degrees, float gear_radius);
float mm_to_degrees (float distance, float gear_radius);
float deg_to_rad(float deg);
float rad_to_deg(float rad);
void pos_mm_to_degree(float* mm_pos, float* deg_pos);

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

void pos_mm_to_degree(float* mm_pos, float* deg_pos)
{
    // (mm, mm) to (degrees, degrees)
    // returns by reference
    deg_pos[0] = mm_to_degrees(mm_pos[0], GEAR_RADIUS_X);
    deg_pos[1] = mm_to_degrees(mm_pos[1], GEAR_RADIUS_Y);
    return;
}