/*
 * pid.c
 *
 *  Created on: Mar 15, 2026
 *      Author: tohar
 */

#include "pid.h"

void P(PIDController* controller, volatile float actual_freq) {
	controller->output += controller->Kp * (controller->target_freq - actual_freq);
}

void I(PIDController* controller, volatile float actual_freq) {
	controller->integral_sum += controller->dt * (controller->target_freq - actual_freq);
	controller->output += controller->Ki * controller->integral_sum;
}

void D(PIDController* controller, volatile float actual_freq) {
	controller->output += controller->Kd * ((actual_freq - controller->last_freq) / controller->dt);
}
