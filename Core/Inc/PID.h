/*
 * PID.h
 *
 *  Created on: Mar 16, 2026
 *      Author: alexi
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#include <stdint.h>

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float target_freq;
    float dt;

    float prev_error;
    float integral;

    float output;

    float out_min;
    float out_max;

    float integral_min;
    float integral_max;
} PIDController;

/**
 * @brief Initialize PID controller fields
 */
void PID_Init(PIDController *pid,
              float Kp,
              float Ki,
              float Kd,
              float dt,
              float out_min,
              float out_max,
              float integral_min,
              float integral_max);

/**
 * @brief Reset dynamic PID state
 */
void PID_Reset(PIDController *pid);

/**
 * @brief Set desired target frequency
 */
void PID_SetTarget(PIDController *pid, float target_freq);

/**
 * @brief Update PID and return signed control output
 *
 * Positive output  -> move motor in direction 1
 * Negative output  -> move motor in direction 2
 * Zero output      -> stop motor
 */
float PID_Update(PIDController *pid, float actual_freq);

/**
 * @brief Clamp helper
 */
float PID_Clamp(float value, float min, float max);

#endif /* INC_PID_H_ */
