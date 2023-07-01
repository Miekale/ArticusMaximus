// Created by Mark Do on 2023-06-25.
//
//

#include "PID.h"
#include <stdio.h>

void PID_controller_log(PID_controller *pid)
{
    printf("integrator %.6f\n", pid->integrator);
    printf("prev_error %.6f \n", pid->prev_error);
    printf("differentiator %.6f \n", pid->differentiator);
    printf("prev_measurement %.6f \n", pid->prev_measurement);
    printf("output %.6f \n", pid->output);
    printf("\n");
}

void PID_controller_init(PID_controller *pid)
{
    // Reset variables
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
    pid->differentiator = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->output = 0.0f;
}

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
