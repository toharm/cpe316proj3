#ifndef INC_PWM_H_
#define INC_PWM_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define PWM_DEFAULT_FREQ_HZ        (50U)
#define PWM_DEFAULT_PERIOD_US      (1000000UL / PWM_DEFAULT_FREQ_HZ)

#define PWM_MAX_DUTY_PC            (100U)
#define PWM_MIN_DUTY_PC            (0U)

typedef enum {
    PWM_OK = 0,
    PWM_INVALID_DUTY,
    PWM_INVALID_PERIOD
} PWMStatus;

/**
 * @brief Initialize TIM3 PWM on two outputs.
 *
 * Example mapping used here:
 *   - TIM3_CH1 -> PA6
 *   - TIM3_CH2 -> PA7
 *
 * Both channels start at 0% duty.
 *
 * @param period_us PWM period in microseconds
 * @return PWM_OK on success
 */
PWMStatus pwm_init(uint32_t period_us);

/**
 * @brief Update duty cycle for output 1 (TIM3_CH1 / PA6)
 * @param duty Duty cycle in percent, 0..100
 * @return PWM_OK on success
 */
PWMStatus update_duty_1(uint8_t duty);

/**
 * @brief Update duty cycle for output 2 (TIM3_CH2 / PA7)
 * @param duty Duty cycle in percent, 0..100
 * @return PWM_OK on success
 */
PWMStatus update_duty_2(uint8_t duty);

#ifdef __cplusplus
}
#endif

#endif /* INC_PWM_H_ */
