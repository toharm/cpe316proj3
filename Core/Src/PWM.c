#include "PWM.h"

/*
 * Example pin mapping for many STM32L4 parts:
 *   PA6 -> TIM3_CH1
 *   PA7 -> TIM3_CH2
 *
 * Verify the exact AF mapping in your specific device datasheet.
 */

/* Internal helper: convert 0..100% duty into CCR value */
static uint32_t pwm_percent_to_ccr(uint8_t duty)
{
    uint32_t arr = TIM3->ARR + 1U;

    if (duty >= 100U) {
        return arr;
    }

    return (arr * duty) / 100U;
}

PWMStatus pwm_init(uint32_t period_us)
{
    if (period_us == 0U) {
        return PWM_INVALID_PERIOD;
    }

    /* Enable peripheral clocks */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

    /* Small delay after enabling clocks */
    __DSB();

    /*
     * Configure PA6 and PA7 as alternate function for TIM3
     * PA6 = TIM3_CH1
     * PA7 = TIM3_CH2
     *
     * On many STM32L4 parts this is AF2.
     * Check your datasheet if your specific part differs.
     */

    /* MODER: alternate function mode */
    GPIOA->MODER &= ~((3UL << (6U * 2U)) | (3UL << (7U * 2U)));
    GPIOA->MODER |=  ((2UL << (6U * 2U)) | (2UL << (7U * 2U)));

    /* OTYPER: push-pull */
    GPIOA->OTYPER &= ~((1UL << 6U) | (1UL << 7U));

    /* PUPDR: no pull */
    GPIOA->PUPDR &= ~((3UL << (6U * 2U)) | (3UL << (7U * 2U)));

    /* OSPEEDR: high speed */
    GPIOA->OSPEEDR &= ~((3UL << (6U * 2U)) | (3UL << (7U * 2U)));
    GPIOA->OSPEEDR |=  ((3UL << (6U * 2U)) | (3UL << (7U * 2U)));

    /* AFRL: AF2 for PA6 and PA7 */
    GPIOA->AFR[0] &= ~((0xFUL << (6U * 4U)) | (0xFUL << (7U * 4U)));
    GPIOA->AFR[0] |=  ((2UL   << (6U * 4U)) | (2UL   << (7U * 4U)));

    /* Stop timer before reconfiguring */
    TIM3->CR1 = 0U;
    TIM3->CCER = 0U;

    /*
     * Set timer tick to 1 MHz (1 us per count)
     * timer_clk / (PSC + 1) = 1,000,000
     */
    uint32_t timer_clk = SystemCoreClock;
    uint32_t psc = (timer_clk / 1000000UL);

    if (psc == 0U) {
        psc = 1U;
    }

    TIM3->PSC = (uint16_t)(psc - 1U);

    /*
     * Edge-aligned PWM:
     * period_us = ARR + 1 at 1 MHz
     */
    TIM3->ARR = period_us - 1U;

    /* Start both channels at 0% duty */
    TIM3->CCR1 = 0U;
    TIM3->CCR2 = 0U;

    /*
     * CH1 = PWM mode 1, preload enable
     * CH2 = PWM mode 1, preload enable
     */
    TIM3->CCMR1 &= ~(
        TIM_CCMR1_OC1M |
        TIM_CCMR1_OC2M |
        TIM_CCMR1_CC1S |
        TIM_CCMR1_CC2S
    );

    TIM3->CCMR1 |=
        (6UL << TIM_CCMR1_OC1M_Pos) |
        TIM_CCMR1_OC1PE |
        (6UL << TIM_CCMR1_OC2M_Pos) |
        TIM_CCMR1_OC2PE;

    /* Auto-reload preload enable */
    TIM3->CR1 |= TIM_CR1_ARPE;

    /* Active high polarity, enable CH1 and CH2 outputs */
    TIM3->CCER &= ~(
        TIM_CCER_CC1P |
        TIM_CCER_CC2P
    );
    TIM3->CCER |= (
        TIM_CCER_CC1E |
        TIM_CCER_CC2E
    );

    /* Force update to load PSC/ARR/CCR */
    TIM3->EGR = TIM_EGR_UG;

    /* Start timer */
    TIM3->CR1 |= TIM_CR1_CEN;

    return PWM_OK;
}

PWMStatus update_duty_1(uint8_t duty)
{
    if (duty > PWM_MAX_DUTY_PC) {
        return PWM_INVALID_DUTY;
    }

    TIM3->CCR1 = pwm_percent_to_ccr(duty);
    return PWM_OK;
}

PWMStatus update_duty_2(uint8_t duty)
{
    if (duty > PWM_MAX_DUTY_PC) {
        return PWM_INVALID_DUTY;
    }

    TIM3->CCR2 = pwm_percent_to_ccr(duty);
    return PWM_OK;
}
