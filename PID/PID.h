//
// Created by Mark Do on 2023-06-25.
//
#ifndef PID_PID_H
#define PID_PID_H

typedef struct
{
    // Controller gains terms
    float kp;
    float ki;
    float kd;

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

void PID_controller_log(PID_controller *pid);
void PID_controller_init(PID_controller *pid);
float PID_controller_update(PID_controller *pid, float set_point, float measurement);

#endif //PID_PID_H
