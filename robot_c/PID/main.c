#include <stdio.h>
#include "PID.h"

int main() {
    // Create controller
    PID_controller pid;
    PID_controller_init(&pid);
    PID_controller_log(&pid);

    pid.sample_time = 0.01;
    pid.tau = 0.01;
    pid.lim_min = -80.0f;
    pid.lim_max = 80.0f;

    pid.kp = 1;
    pid.ki = 1;
    pid.kd = 0.01;

    //Update controller
    PID_controller_update(&pid, 10, 0);
    PID_controller_log(&pid);

    PID_controller_update(&pid, 10, 3);
    PID_controller_log(&pid);

    PID_controller_update(&pid, 10, 8);
    PID_controller_log(&pid);

    PID_controller_update(&pid, 10, 13);
    PID_controller_log(&pid);

    return 0;
}
