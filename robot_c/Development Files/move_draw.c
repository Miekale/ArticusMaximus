// Functions required to move and draw robot
float calc_angle(float* pos_0, float* pos_1);
void calc_motor_power(float angle, int max_power, int* motor_powers);
void move_pen(float* pos_0, float* pos_1, bool draw, int max_draw_power, int max_move_power, int pen_distance);

void calc_motor_power(float angle, int max_power, int* motor_powers)
{
    /*
    Calculates x motor and y motor power from input angle,
    scaled by max power. Returns by reference

    PARAMETERS
    ----------
    angle: float, radians, horizontal from [1,0] of between start and end points
    max_power: int, positive < 100, maximum power allowed.
    motor_powers: int array[power x, power y], reference output.

    RETURNS
    -------
    motor_powers: int array[power x, power y]

    */
    assert(angle >= 0 || angle <= 180 || max_power <= 100 || max_power > 0);
    angle = deg_to_rad(angle);
    motor_powers[0] = round(max_power * cos(angle));
    motor_powers[1] = round(max_power * sin(angle));

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
    else if (delta_y < 0 && delta_x > 0)
    {
        angle += 360;
    }
    // 1st Quadrant do nothing
    return angle;
}
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
    int motor_powers[2] = {0,0};
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
    // Check motor encoders against angle-converted position
    float encoder_target[2] ={0,0};
    pos_mm_to_degree(pos_1, encoder_target);
    // 1st Quadrant
    if (angle < 90)
    {
        while (nMotorEncoder[motorA] < encoder_target[0] || nMotorEncoder[motorD] < encoder_target[1])
        {}
    }
        // 2nd Quadrant
    else if (angle < 180)
    {
        while (nMotorEncoder[motorA] > encoder_target[0] || nMotorEncoder[motorD] < encoder_target[1])
        {}
    }
        // 3rd Quadrant
    else if (angle < 270)
    {
        while (nMotorEncoder[motorA] > encoder_target[0] || nMotorEncoder[motorD] > encoder_target[1])
        {}
    }
        // 4th Quadrant
    else
    {
        while (nMotorEncoder[motorA] < encoder_target[0] || nMotorEncoder[motorD] > encoder_target[1])
        {}
    }
    motor[motorA] = motor[motorD] = 0;

    if (draw)
    {
        pen_up();
    }

    return;

}