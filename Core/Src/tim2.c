/*
 * tim2.c
 *
 *  Created on: Feb 9, 2026
 *      Author: alexi
 */

#include "tim2.h"
#include "main.h"

void init_tim2(){
	TIM2->PSC = 0;
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);
	TIM2->ARR = 1999; //I want 16kHz. from 32Mhz, that's 2000
	TIM2->CR2 &= ~TIM_CR2_MMS;
	TIM2->CR2 |= (0x2U << TIM_CR2_MMS_Pos); // 0010 for trigger output
	TIM2->EGR |= TIM_EGR_UG;                  // force update event, load registers
	TIM2->SR  &= ~TIM_SR_UIF;
	TIM2->CR1 |= TIM_CR1_CEN;
}

