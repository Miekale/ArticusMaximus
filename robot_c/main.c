#include "PC_FileIO.c"

// Global Hardware Constants
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
void pos_degree_to_mm(float* mm_pos, float* deg_pos);
// PID CONTROLLER FUNCTIONS
void PID_controller_log(PID_controller *pid);
void PID_Controller_reset(PID_controller *pid);
float PID_controller_update(PID_controller *pid, float set_point, float measurement);
void draw_PID(PID_controller* pid_x, PID_controller* pid_y, float* target_pos, bool draw);


// ROBOTC MOVEMENT FUNCTIONS
void initialize_sensors();
void get_current_pos(float* mm_pos);
void zero(float* pos);
void pen_up();
void pen_down();
float calc_angle(float* pos_0, float* pos_1);
void calc_motor_power(float angle, int max_power, float* motor_powers);
void draw_no_PID(float* target_pos, bool draw, int max_draw_power, int max_move_power);

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
}

void pos_degree_to_mm(float* mm_pos, float* deg_pos) {
    mm_pos[0] = degrees_to_mm(deg_pos[0], GEAR_RADIUS_X);
    mm_pos[1] = degrees_to_mm(deg_pos[1], GEAR_RADIUS_Y);
}

// Printing PID Controller values
void PID_controller_log(PID_controller *pid)
{
    displayString(5, "integrator %.6f\n", pid->integrator);
    displayString(10, "prev_error %.6f \n", pid->prev_error);
    displayString(15, "differentiator %.6f \n", pid->differentiator);
    displayString(20, "prev_measurement %.6f \n", pid->prev_measurement);
    displayString(25, "output %.6f \n", pid->output);
    displayString(30, "\n");
}
// Reset PID values
void PID_Controller_reset(PID_controller *pid)
{
    // Reset variables
    pid->integrator = 0.0;
    pid->prev_error = 0.0;
    pid->differentiator = 0.0;
    pid->prev_measurement = 0.0;
    pid->output = 0.0;
}
// Update PID values
float PID_controller_update(PID_controller *pid, float set_point, float measurement)
{
    // Error Signal
    float error = set_point - measurement;

    // Proportional
    float proportional = pid->kp * error;

    // Integral
    pid->integrator += 0.5 * pid->ki * pid->sample_time * (error + pid->prev_error);

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
    pid->differentiator = (2.0 * pid->kd *(measurement - pid->prev_measurement) + (2.0 * pid->tau - pid->sample_time) * pid->differentiator) / (2.0 * pid->tau + pid->sample_time);

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
// TODO: GET REAL SENSOR PORTS AND INITIALIZE
void initialize_sensors()
{
    //touch x
    SensorType[S1] = sensorEV3_Touch;
    //touch y
    SensorType[S2] = sensorEV3_Touch;
    SensorType[S3] = sensorEV3_Color;

}
// Get current pen position in mm
void get_current_pos(float* mm_pos)
{
    float deg_pos[2] = {0, 0};
    deg_pos[0] = nMotorEncoder[motorA];
    deg_pos[1] = nMotorEncoder[motorD];
    pos_degree_to_mm(mm_pos, deg_pos);
}
// Zero pen x, y, z
void zero(float* pos)
{
    int const speed_initial = 20;
    int const speed_final = 10;
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
void pen_up()
{
    // Setting motor to run forwards until distance is pen distance away from the page
    motor[motorD] = 25;
    while(PEN_DISTANCE > degrees_to_mm(nMotorEncoder[motorD], GEAR_RADIUS_Z))
    {}
    motor[motorD] = 0;
}
// Moves pen away from page
void pen_down()
{
    // Setting motor runs backwards until distance is backwards to 0mm
    motor[motorD] = -25;
    while(0 < degrees_to_mm(nMotorEncoder[motorD], GEAR_RADIUS_Z))
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

void draw_no_PID(float* target_pos, bool draw, int max_draw_power, int max_move_power)
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
    float const POS_TOL = 0.1;  // pen move within 0.1mm of actual target

    // Initialize starting positions
    float current_pos[2] = {0, 0};
    get_current_pos(current_pos);

    // Drawing mode
    if (draw)
    {
        // Get motor powers
        float motor_powers[2] = {0,0};
        float angle = calc_angle(current_pos, target_pos);
        calc_motor_power(angle, max_draw_power, motor_powers);

        pen_down();

        // Move motors at power
        motor[motorA] = motor_powers[0];
        motor[motorD] = motor_powers[1];

        // Keep moving motor until end position reached
        while ((abs(current_pos[0] - target_pos[0]) > POS_TOL) || (abs(current_pos[1] - target_pos[1]) > POS_TOL))
        {
            get_current_pos(current_pos);
        }

        pen_up();
    }
    // Moving mode
    else
    {
        // Move x motor until target
        motor[motorA] = max_move_power;
        while ((abs(current_pos[0] - target_pos[0]) > POS_TOL))
        {
            get_current_pos(current_pos);
        }
        motor[motorA] = 0;
        motor[motorD] = max_move_power;
        // Move y motor until target
        while ((abs(current_pos[0] - target_pos[1]) > POS_TOL))
        {
            get_current_pos(current_pos);
        }
        motor[motorD] = 0;
    }
    return;
}

void draw_PID(PID_controller* pid_x, PID_controller* pid_y, float* target_pos, bool draw)
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
    // If moving only no PID needed
    if (!draw)
    {
        draw_no_PID(target_pos, false, 80, 80);
        return;
    }

    // Pen move until within 0.1mm of actual target
    float const POS_TOL = 0.1;

    // Initialize starting positions
    float motor_powers[2] = {0,0};
    float starting_pos[2] = {0, 0};
    float current_pos[2] = {0, 0};
    get_current_pos(current_pos);
    starting_pos[0] = current_pos[0];
    starting_pos[1] = current_pos[1];

    pen_down();

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
    pen_up();

}

/*
// Just to test non RobotC Functions
int main()
{
    // MOTOR POWER TESTS
    int motor_powers[2] = {};
    calc_motor_power(45, 100, motor_powers);
    assert(motor_powers[0] == 71);
    assert(motor_powers[1] == 71);

    calc_motor_power(30, 100, motor_powers);
    assert(motor_powers[0] == 87);
    assert(motor_powers[1] == 50);

    calc_motor_power(135, 100, motor_powers);
    assert(motor_powers[0] == -71);
    assert(motor_powers[1] == 71);

    calc_motor_power(225, 100, motor_powers);
    assert(motor_powers[0] == -71);
    assert(motor_powers[1] == -71);

    calc_motor_power(315, 100, motor_powers);
    assert(motor_powers[0] == 71);
    assert(motor_powers[1] == -71);

    // ANGLE CALCULATOR TESTS
    float x_positions[6] = {static_cast<float>(sqrt(3))/2, 1, -1, -1, 0.5, 1};
    float y_positions[6] = {0.5, 1, 1, -1, static_cast<float>(-sqrt(3))/2, -1};
    float origin[2] = {0,0};
    float angles[6] = {};
    for (int i = 0; i < 6; i++)
    {
        float pos_1[2] = {x_positions[i], y_positions[i]};
        angles[i] = calc_angle(origin, pos_1);
        cout << angles[i] << endl;
    }
}
*/

void non_PID_main()
{
    // ---- INITIALIZATION ---- //
    // Initialize Sensors
    initialize_sensors();

    // Input File Validation
    TFileHandle fin;
    bool fileOkay = openReadPC(fin, "instructions.txt");
    if (!fileOkay) {
        displayString(5, "FILE READ ERROR!");
        wait1Msec(3000);
        return;
    }

    // Initialize position and zero pen
    float pen_pos[2] = {0,0};
    zero(pen_pos);

    // ---- DRAWING LOOP ---- //
    // Read each contour
    string contour_name = "";
    while (readTextPC(fin, contour_name))
    {
        int contour_size = 0;
        readIntPC(fin, contour_size);
        for (int point = 0; point < contour_size; point++)
        {
            // Determine if D (draw) or M (move)
            bool is_draw = false;
            string move_or_draw = "";
            readTextPC(fin, move_or_draw);
            if (move_or_draw == "D")
            {
                is_draw = true;
            }

            // Get target location
            float next_point[2] = {0,0};
            readFloatPC(fin, next_point[0]);
            readFloatPC(fin, next_point[1]);

            // Move to target location
            draw_no_PID(next_point, is_draw, 80, 80);
        }
    }
    // close file
    closeFilePC(fin);
}
// Actual main
task main()
{
		float angle = 0;
		int distance_time = 500;
		while (angle < 360){
			float motor_powers[2];
			motor_powers[0] = 0;
			motor_powers[1] = 0;

			int max_power = 50;
			calc_motor_power(angle, max_power, motor_powers);

			int intMotorPowers[2] = {round(motor_powers[0]), round(motor_powers[1])};

			motor[motorA] = intMotorPowers[0];
			motor[motorD] = intMotorPowers[1];
			wait1Msec(distance_time);
			motor[motorA] = -intMotorPowers[0];
			motor[motorD] = -intMotorPowers[1];
			wait1Msec(distance_time);
			angle += 30;
		}
		/*
		motor[motorA] = motor[motorD] = 30;
		wait1Msec(1000);
		motor[motorA] = motor[motorD] = -30;
		wait1Msec(1000);
		pen_down();
		motor[motorA] = motor[motorD] = 30;
		wait1Msec(1000);
		motor[motorA] = 0;
		motor[motorD] = -30;
		wait1Msec(1000);
		motor[motorD] = 0;
		motor[motorA] = motor[motorD] = -30;
		wait1Msec(1000);
		motor[motorD] = 30;
		wait1Msec(1000);
		motor[motorD] = 0;
		motor[motorA] = 30;
		wait1Msec(1000);
		*///pen_up();

		/*
    // ---- INITIALIZATION ---- //
    // Initialize Sensors
    initialize_sensors();

    // File Validation
    TFileHandle fin;
    bool input_file_okay = openReadPC(fin, "instructions.txt");
    if (!input_file_okay) {
        displayString(5, "FILE READ ERROR!");
        wait1Msec(5000);
        return;
    }
    TFileHandle fout;
    bool output_file_okay = openReadPC(fin, "output.txt");
    if (!output_file_okay) {
        displayString(5, "FILE WRITE ERROR!");
        wait1Msec(5000);
        return;
    }

    // Initialize position and zero pen
    float pen_pos[2] = {0,0};
    zero(pen_pos);

    // Create controller
    PID_controller pid_x;
    PID_controller pid_y;
    PID_Controller_reset(&pid_x);
    PID_Controller_reset(&pid_y);
    writeTextPC(fout, "PID controllers created");
    writeEndlPC(fout);

    // TODO: Tune Low-pass filter tau and calculate sample time
    pid_x.sample_time = pid_y.sample_time = 0.005;
    pid_x.speed = pid_y.speed = 0.2;
    pid_x.tau = 0.00;
    pid_x.lim_min = pid_y.lim_min = -80.0;
    pid_x.lim_max = pid_y.lim_max = 80.0;
    writeTextPC(fout, "PID values assigned");
    writeEndlPC(fout);


    // TODO: Tune Constants
    pid_x.kp = 1;
    pid_x.ki = 0;
    pid_x.kd = 0;
    pid_y.kp = 1;
    pid_y.ki = 0;
    pid_y.kd = 0;
    writeTextPC(fout, "PID k-values assigned");
    writeEndlPC(fout);

    // ---- DRAWING LOOP ---- //
    // Read each contour
    string contour_name = "";
    while (readTextPC(fin, contour_name))
    {
        writeTextPC(fout, "Processing Contour #");
        writeTextPC(fout, contour_name);
        writeEndlPC(fout);

        int contour_size = 0;
        readIntPC(fin, contour_size);
        for (int point = 0; point < contour_size; point++)
        {
            // Determine if D (draw) or M (move)
            bool is_draw = false;
            string move_or_draw = "";
            readTextPC(fin, move_or_draw);

            if (move_or_draw == "D")
            {
                is_draw = true;
            }

            // Get target location
            float next_point[2] = {0,0};
            readFloatPC(fin, next_point[0]);
            readFloatPC(fin, next_point[1]);

            // Move to target location
            writeTextPC(fout, "Moving to point: ");
            writeFloatPC(fout, next_point[0]);
            writeTextPC(fout, " ");
            writeFloatPC(fout, next_point[1]);
            writeEndlPC(fout);

            draw_PID(&pid_x, &pid_y, next_point, is_draw);
        }
    }
    // close file
    closeFilePC(fin);
    */
}
