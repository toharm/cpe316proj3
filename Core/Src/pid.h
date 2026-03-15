/*
 * pid.h
 *
 *  Created on: Mar 15, 2026
 *      Author: tohar
 */

#ifndef SRC_PID_H_
#define SRC_PID_H_

#include<stdint.h>

typedef struct {
	volatile float Kp;
	volatile float Ki;
	volatile float Kd;
	volatile float target_freq;
	volatile float last_freq;
	volatile float dt;
	volatile float integral_sum;
	volatile float output;
} PIDController;

void P(PIDController*, volatile float);
void I(PIDController*, volatile float);
void D(PIDController*, volatile float);

#endif /* SRC_PID_H_ */
