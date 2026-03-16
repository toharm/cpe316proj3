/**
 * @file PWM.c
 * @brief Implementation of a tiny PWM helper for TIM2 CH1 (PA5) on STM32L4.
 * @author Noah
 * @version 1.1.0b+BasedOnv1.0.0Commit402f325
 * @author GitHub Copilot, GPT-4 mini, Ask mode
 * @author GitHub Copilot, GPT-5 mini, Agent mode
 *
 * The implementation programs TIM2/TIM3 to use a 1 MHz timebase (1 µs tick),
 * sets ARR and CCRx according to the requested period and duty cycle, and
 * starts the timer. GPIOA pins PA5 (AF1 TIM2_CH1) and PA7 (AF2 TIM3_CH2) are
 * configured by the respective setup functions.
 */

#include "PWM.h"
#include "stm32l4xx_hal_tim.h"


/**
 * This routine enables the GPIOA and TIM2 peripheral clocks, configures PA5
 * for alternate function TIM2_CH1 (AF1), programs the timer PSC so that the
 * timer ticks at 1 MHz (1 µs resolution), sets ARR and CCR1 according to the
 * requested period and duty cycle, configures the channel in PWM mode 1 with
 * preload enabled, forces an update to load shadow registers, and starts the
 * counter.
 */
PWMStatus setupPWM(uint32_t period_us, uint_fast16_t dutyCycle) {
	// Check duty cycle
	if (dutyCycle > PWM_MAX_DC_PC)
		return PWM_DC_TOO_HIGH;
	if (dutyCycle < PWM_MIN_DC_PC)
		return PWM_DC_TOO_LOW;

	// Enable GPIOA clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Configure PA5 as AF1 (TIM2_CH1)
	GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
	GPIOA->MODER |= GPIO_MODER_MODE5_1; // AF mode (10)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT5; // push-pull
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_Msk;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5; // high speed
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL5_Msk;
	GPIOA->AFR[0] |= (GPIO_AF1_TIM2 << GPIO_AFRL_AFSEL5_Pos);

	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	// Configure timer for 1 MHz tick (1 us resolution)
	uint32_t sysclk = SystemCoreClock; // requires CMSIS/HAL system init
	uint32_t psc = (sysclk / 1000000UL);
	if (psc == 0)
		psc = 1;
	TIM2->PSC = (uint16_t) (psc - 1);

	// For center-aligned mode the effective ARR is half period ticks
	uint32_t halfPeriod_us = (period_us / 2U);
	if (halfPeriod_us == 0)
		halfPeriod_us = 1; // avoid ARR==0
	TIM2->ARR = (uint32_t) (halfPeriod_us - 1U);

	// Set PWM duty (CCR1) using the half-period as the reference
	TIM2->CCR1 = (uint32_t) ((halfPeriod_us * dutyCycle) / 100U);

	// Configure channel 1 as PWM mode 1 and enable preload
#ifndef TIM_OCMODE_PWM1
#define TIM_OCMODE_PWM1 (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#endif
	// Clear OC1M bits then set PWM1
	TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M);
	TIM2->CCMR1 |= TIM_OCMODE_PWM1;
	// Enable CCR1 preload (OC1PE), not OC1FE
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;

	// Enable auto-reload preload
	TIM2->CR1 |= TIM_CR1_ARPE;

	// Enable center-aligned mode 1 (CMS = 01)
	TIM2->CR1 &= ~TIM_CR1_CMS;
	TIM2->CR1 |= TIM_CR1_CMS_0;

	// Clear polarity bit for active-high and enable output
	TIM2->CCER &= ~(TIM_CCER_CC1P); // clear active high
	TIM2->CCER |= TIM_CCER_CC1E;    // enable CH1 output

	// Force update to load prescaler/ARR/CCR1 into shadow registers
	TIM2->EGR = TIM_EGR_UG;

	// Start timer
	TIM2->CR1 |= TIM_CR1_CEN;

	return PWM_OK;
}

/**
 * Checks the dutyCycle and and programs TIM2->CCR1. This function is safe to call while the timer is running; CCR1
 * preload is enabled so the new value will take effect at the next update
 * event.
 */
PWMStatus setPWMDutyCyclePC(uint32_t dutyCycle) {
	uint32_t halfPeriod = TIM2->ARR + 1; // ARR holds half-period in center-aligned mode

	// Check duty cycle
	if (dutyCycle > halfPeriod)
		return PWM_DC_TOO_HIGH;
	if (dutyCycle < 0)
		return PWM_DC_TOO_LOW;
	// Else: Set PWM duty (CCR1) using the half-period as the reference
	TIM2->CCR1 = dutyCycle;

	return PWM_OK;
}

