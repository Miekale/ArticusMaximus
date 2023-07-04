#include <stdio.h>

// Global Constants
float const GEAR_RADIUS_X = 1;
float const GEAR_RADIUS_Y = 1;
float const GEAR_RADIUS_Z = 1;
float const PEN_DISTANCE = 5;

// PID Controller
typedef struct
{
    // Controller gains terms
    float kp;
    float ki;
    float kd;

    // Controller motion profile
    float speed;

    // Derivative low pass filter time constant
    float tau;

    // Output Limits
    float lim_min;
    float lim_max;

    // Sample Time (seconds)
    float sample_time;

    // Controller stored vars
    float integrator;
    float prev_error;
    float differentiator;
    float prev_measurement;

    // Controller Output Var
    float output;

} PID_controller;

// ------ FUNCTION PROTOTYPES ------ //
// CONVERSION FUNCTIONS
float degrees_to_mm (int degrees, float gear_radius);
float mm_to_degrees (float distance, float gear_radius);
float deg_to_rad(float deg);
float rad_to_deg(float rad);
void pos_mm_to_degree(float* mm_pos, float* deg_pos);
// PID CONTROLLER FUNCTIONS
void PID_controller_log(PID_controller *pid);
void PID_controller_init(PID_controller *pid);
float PID_controller_update(PID_controller *pid, float set_point, float measurement);
void move_pen_with_PID(PID_controller *pid, float* pos_0, float* pos_1, bool draw,
                       int max_draw_power, int max_move_power);
// ROBOTC MOVEMENT FUNCTIONS
void initialize_sensors();
void get_current_pos(float* pos);
void zero(float* pos);
void pen_up();
void pen_down();
float calc_angle(float* pos_0, float* pos_1);
void calc_motor_power(float angle, int max_power, float* motor_powers);
void move_pen(float* pos_0, float* pos_1, bool draw, int max_draw_power, int max_move_power);

// ------ FUNCTION DEFINITIONS ------ //

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
// Converts pos(x,y) from mm to degrees, returns by reference
void pos_mm_to_degree(float* mm_pos, float* deg_pos)
{
    deg_pos[0] = mm_to_degrees(mm_pos[0], GEAR_RADIUS_X);
    deg_pos[1] = mm_to_degrees(mm_pos[1], GEAR_RADIUS_Y);
    return;
}

// Printing PID Controller values
void PID_controller_log(PID_controller *pid)
{
    printf("integrator %.6f\n", pid->integrator);
    printf("prev_error %.6f \n", pid->prev_error);
    printf("differentiator %.6f \n", pid->differentiator);
    printf("prev_measurement %.6f \n", pid->prev_measurement);
    printf("output %.6f \n", pid->output);
    printf("\n");
}
// Reset PID values
void PID_controller_init(PID_controller *pid)
{
    // Reset variables
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
    pid->differentiator = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->output = 0.0f;
}
// Update PID values
float PID_controller_update(PID_controller *pid, float set_point, float measurement)
{
    // Error Signal
    float error = set_point - measurement;

    // Proportional
    float proportional = pid->kp * error;

    // Integral
    pid->integrator += 0.5f * pid->ki * pid->sample_time * (error + pid->prev_error);

    // Anti-Windup with dynamic integrator clamping
    float lim_min_int = 0, lim_max_int = 0;

    if (pid->lim_max > proportional)
    {
        lim_max_int = pid->lim_max - proportional;
    }
    if (pid->lim_min < proportional)
    {
        lim_min_int = pid->lim_min - proportional;
    }

    // Limit integrator value
    if (pid->integrator < lim_min_int)
    {
        pid->integrator = lim_min_int;
    }
    if (pid->integrator > lim_max_int)
    {
        pid->integrator = lim_min_int;
    }

    // Derivative (low pass filter)
    pid->differentiator = (2.0f * pid->kd *(measurement - pid->prev_measurement)
                           + (2.0f * pid->tau - pid->sample_time) * pid->differentiator)
                          / (2.0f * pid->tau + pid->sample_time);

    // Compute output
    pid->output = proportional + pid->integrator + pid->differentiator;
    if (pid->output < pid->lim_min)
    {
        pid->output  = pid->lim_min;
    }
    else if (pid->output > pid->lim_max)
    {
        pid->output  = pid->lim_max;
    }

    pid->prev_error = error;
    pid->prev_measurement = measurement;

    return pid->output;
}

// Init Sensors
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
// Get current pen position in mm
// TODO: WRITE POS_DEG_TO_MM AND IMPLEMENT HERE
void get_current_pos(float* pos)
{
    pos[0] = nMototEncoder(motorA);
    pos[1] = nMotorEncoder(motorD);
}
// Zero pen x, y, z
void zero(float* pos)
{
    //zeroing speeds
    int speed_initial = 25;
    int speed_final = 10;

    //testing if on the sensor initially and moving off the sensor
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
    //initialize inordinate 0, 0
    pos[0] = 0;
    pos[1] = 0;
}
// Moves pen into contact with page
void pen_down()
{
    // Setting motor to run forwards until distance is pen distance away from the page
    motor[motorD] = 25;
    while(PEN_HEIGHT > degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
    {}
    motor[motorD] = 0;
}
// Moves pen away from page
void pen_up()
{
    // Setting motor runs backwards until distance is backwards to 0mm
    motor[motorD] = -25;
    while(0 < degrees_to_mm(nMotorEncoder[motorD], z_axis_gear_radius))
    {}
    motor[motorD] = 0;
}

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

void move_pen(float* pos_0, float* pos_1, bool draw, int max_draw_power, int max_move_power)
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
    float motor_powers[2] = {0,0};


    // Drawing mode
    if (draw)
    {
        calc_motor_power(angle, max_draw_power, motor_powers);
        pen_down();
    }
    else
    {
        calc_motor_power(angle, max_move_power, motor_powers);
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

void move_pen_with_PID(PID_controller* pid_x, PID_controller* pid_y, float* target_pos, bool draw)
{
    /* Controls x motor and y motor to move pen from starting position
     to ending position using PID controller.

     PARAMETERS
     ----------
     pid_x: PID Controller struct for x-direction motor
     pid_y: PID Controller struct for y-direction motor
     target_pos: float array[x,y], target position.
     max_draw_power: int <100, maximum motor power allowed while drawing
     max_move_power: int <100, maximum motor power allowed while moving

     RETURNS
     -------
     */
    float const POS_TOL = 0.1;  // pen move within 0.1mm of actual target

    // Initialize starting positions
    float motor_powers[2] = {0,0};
    float starting_pos[2] = {0};
    float current_pos[2] = {0};
    get_current_pos(current_pos);
    starting_pos[0] = current_pos[0];
    starting_pos[1] = current_pos[1];

    // Drawing mode
    if (draw)
    {
        pen_down();
    }

    // Motion profile
    float x_f = target_pos[0];
    float x_i = starting_pos[0];
    float y_f = target_pos[1];
    float y_i = starting_pos[1];

    // PID Loop
    time1[T1] = 0;
    while ((abs(current_pos[0] - x_f) > POS_TOL) || (abs(current_pos[1] - y_f) > POS_TOL))
    {
        // get next point on motion profile
        float t = time1[T1] * pid_x->speed;
        float x_t = x_i + (x_f - x_i) * t;
        float y_t = y_i + (y_f - y_i) * t;
        // update PID controllers
        PID_controller_update(pid_x, x_t, current_pos[0]);
        PID_controller_update(pid_y, y_t, current_pos[1]);
        motor_powers[0] = pid_x->output;
        motor_powers[1] = pid_x->output;
        wait1Msec(pid_x->sample_time);
        // update current position for loop condition
        get_current_pos(current_pos);
    }

    // Turn off motors once target reached
    motor[motorA] = motor[motorD] = 0;
    if (draw)
    {
        pen_up();
    }
}
// Just to test non RobotC Functions
int main()
{
    // MOTOR POWER TESTS
//    int motor_powers[2] = {};
//    calc_motor_power(45, 100, motor_powers);
//    assert(motor_powers[0] == 71);
//    assert(motor_powers[1] == 71);
//
//    calc_motor_power(30, 100, motor_powers);
//    assert(motor_powers[0] == 87);
//    assert(motor_powers[1] == 50);
//
//    calc_motor_power(135, 100, motor_powers);
//    assert(motor_powers[0] == -71);
//    assert(motor_powers[1] == 71);
//
//    calc_motor_power(225, 100, motor_powers);
//    assert(motor_powers[0] == -71);
//    assert(motor_powers[1] == -71);
//
//    calc_motor_power(315, 100, motor_powers);
//    assert(motor_powers[0] == 71);
//    assert(motor_powers[1] == -71);

    // ANGLE CALCULATOR TESTS
//    float x_positions[6] = {static_cast<float>(sqrt(3))/2, 1, -1, -1, 0.5, 1};
//    float y_positions[6] = {0.5, 1, 1, -1, static_cast<float>(-sqrt(3))/2, -1};
//    float origin[2] = {0,0};
//    float angles[6] = {};
//    for (int i = 0; i < 6; i++)
//    {
//        float pos_1[2] = {x_positions[i], y_positions[i]};
//        angles[i] = calc_angle(origin, pos_1);
//        cout << angles[i] << endl;
//    }



}

// Actual main
task main()
{
    // ---- INITIALIZATION LOOP ---- //
    // Initialize Sensors
    initialize_sensors();

    // Input File Validation
    TFileHandle fin;
    bool fileOkay = openReadPC(fin, "instructions.txt");
    if (!fileOkay) {
        displayString(5, "FILE READ ERROR!");
        wait1MSec(3000);
        return;
    }

    // Initialize position and zero pen
    float pen_pos[2] = {0,0};
    zero(pen_pos);

    // Create controller
    PID_controller pid_x;
    PID_controller pid_y;
    PID_controller_init(&pid_x);
    PID_controller_init(&pid_y);
    // TODO: Tune Low-pass filter tau and calculate sample time
    pid_x.sample_time = pid_y.sample_time = 0.01;
    pid_x.speed = pid_y.speed = 0.2;
    pid_x.tau = 0.00;
    pid_x.lim_min = pid_y.lim_min = -80.0;
    pid_x.lim_max = pid_y.lim_max = 80.0;
    // TODO: Tune Constants
    pid_x.kp = 1;
    pid_x.ki = 0;
    pid_x.kd = 0;
    pid_y.kp = 1;
    pid_y.ki = 0;
    pid_y.kd = 0;

    // ---- DRAWING LOOP ---- //
    // Read each contour
    string contour_name = "";
    while (readTextPC(fin, contour_name)
    {
        int contour_size = 0;
        readIntPC(fin, contour_size);
        for (int point = 0; point < countour_size; point++)
        {
            // Determine if D (draw) or M (move)
            bool is_draw = false;
            string move_or_draw = "";
            readStringPC(fin, move_or_draw);
            if move_or_draw == "D"
            {
                is_draw = true;
            }
            // Get target location
            int next_point[2] = {0,0};
            readIntPC(fin, next_point[0]);
            readIntPC(fin, next_point[1]);
            // Move to target location
            move_pen_with_PID(pid_x, pid_y, next_point, draw);
        }
    }
    // close file
    closeFilePC(fin);
}