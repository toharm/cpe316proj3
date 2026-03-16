/**
 * @file PWM.h
 * @brief Simple PWM helper for TIM2 CH1 (PA5) and TIM2 CH2 (PA1) on STM32L4 series.
 * @version 1.1.0b+BasedOnv1.0.0Commit402f325
 * @author Noah
 * @author GitHub Copilot, GPT-4 mini, Ask mode
 * @author GitHub Copilot, GPT-5 mini, Agent mode
 *
 * This header declares stuff to configure TIM2 to produce a PWM
 * waveform on PA5 (TIM2_CH1) and PA1 (TIM2_CH2) using a 1 MHz timer tick
 * (1us resolution). The implementation expects SystemCoreClock to be set by
 * the system/HAL.
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

/* Config */
#define PWM_DEFAULT_FREQ_Hz (50)
#define PWM_DEFAULT_PERIOD_US (1000000UL / PWM_DEFAULT_FREQ_Hz) // 20_000 us for 50 Hz
#define PWM_DEFAULT_DUTY_CYCLE_PC (50) // percent

#define PWM_MAX_DC_PC (100)
#define PWM_MIN_DC_PC (0)

typedef enum {
	PWM_OK, /**< Operation succeeded */
	PWM_DC_TOO_HIGH, /**< Requested duty cycle above PWM_MAX_DC_PC */
	PWM_DC_TOO_LOW /**< Requested duty cycle below PWM_MIN_DC_PC */
} PWMStatus;

/**
 * @brief Configure TIM2 channel 1 (PA5) for PWM output (center-aligned).
 *
 * This function configures PA5 as TIM2_CH1 alternate function, enables the
 * TIM2 clock, and programs the timer to use a 1 MHz timebase (1 microsecond
 * tick) in center-aligned counting mode. For center-aligned operation the
 * effective period in timer ticks is 2*(ARR+1), therefore ARR is set to
 * (period_us/2) - 1 and CCR1 is computed relative to the half-period (ARR+1).
 *
 * @param period_us PWM period in microseconds (e.g. 20000 for 50 Hz)
 * @param dutyCycle Duty cycle in percent (0..100)
 * @return PWM_OK on success, PWM_DC_TOO_HIGH or PWM_DC_TOO_LOW for invalid duty
 *         cycle values.
 *
 * @note SystemCoreClock must be valid before calling this function.
 */
PWMStatus setupPWM(uint32_t period_us, uint_fast16_t dutyCycle);

/**
 * @brief Set PWM duty cycle (raw integer)
 *
 * Updates TIM2->CCR1 so the PWM duty matches the requested value. This uses
 * the currently programmed ARR to compute the pulse width.
 *
 * @param dutyCycle Duty cycle in ticks
 * @return PWM_OK on success, PWM_DC_TOO_HIGH or PWM_DC_TOO_LOW for invalid duty
 *         cycle values.
 */
PWMStatus setPWMDutyCyclePC(uint32_t dutyCycle);

/**
 * @brief Set PWM duty cycle (floating-point percent)
 *
 * Same as setPWMDutyCyclePC but accepts a float for finer-grained values.
 *
 * @param dutyCycle Duty cycle in percent (0.0 .. 100.0)
 * @return PWM_OK on success, PWM_DC_TOO_HIGH or PWM_DC_TOO_LOW for invalid duty
 *         cycle values.
 */
//PWMStatus setPWMDutyCyclePC2(float dutyCycle);

#endif /* INC_PWM_H_ */
