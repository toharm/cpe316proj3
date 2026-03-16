/*
 * pid.c
 *
 *  Created on: Mar 16, 2026
 *      Author: tohar
 */

#include "pid.h"

float PID_Clamp(float value, float min, float max)
{
    if (value > max) {
        return max;
    }
    if (value < min) {
        return min;
    }
    return value;
}

void PID_Init(PIDController *pid,
              float Kp,
              float Ki,
              float Kd,
              float dt,
              float out_min,
              float out_max,
              float integral_min,
              float integral_max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->target_freq = 0.0f;
    pid->dt = dt;

    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;

    pid->out_min = out_min;
    pid->out_max = out_max;

    pid->integral_min = integral_min;
    pid->integral_max = integral_max;
}

void PID_Reset(PIDController *pid)
{
    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

void PID_SetTarget(PIDController *pid, float target_freq)
{
    pid->target_freq = target_freq;
}

float PID_Update(PIDController *pid, float actual_freq)
{
    float error;
    float derivative;
    float p_term;
    float i_term;
    float d_term;
    float output;

    error = 1200.0f * log2f(pid->target_freq / actual_freq);

    //error = pid->target_freq - actual_freq;

    /* Integral term */
    pid->integral += error * pid->dt;
    pid->integral = PID_Clamp(pid->integral, pid->integral_min, pid->integral_max);

    /* Derivative term */
    derivative = (error - pid->prev_error) / pid->dt;

    /* PID terms */
    p_term = pid->Kp * error;
    i_term = pid->Ki * pid->integral;
    d_term = pid->Kd * derivative;

    /* Total output */
    output = p_term + i_term + d_term;

    /* Clamp to allowed output range */
    output = PID_Clamp(output, pid->out_min, pid->out_max);

    pid->output = output;
    pid->prev_error = error;

    return output;
}
