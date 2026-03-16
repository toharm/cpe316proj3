/*
 * adc_helpers.c
 *
 *  Created on: Mar 11, 2026
 *      Author: alexi
 */


#include <stdint.h>
#include <string.h>
#include "stm32l4xx.h"
#include "adc_helpers.h"
#include "main.h"


static volatile uint16_t adc_buffer[BUFFER_SIZE];

void ADC_Init(void)
{
    // clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE;
    ADC123_COMMON->CCR |=  ADC_CCR_CKMODE_0;   // HCLK/1 to ADC
    // PA0 = ADC12_IN5
    GPIOA->MODER |= GPIO_MODER_MODE0;
    GPIOA->PUPDR &= ~(3U << (2*0));
    GPIOA->ASCR  |= GPIO_ASCR_ASC0;

    // ADC power-up
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;

    // short regulator startup delay here
    HAL_Delay(1);
    // calibrate
    // Make sure ADC is disabled
    if (ADC1->CR & ADC_CR_ADEN) {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN) {}
    }
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {}

    // regular channel config: one conversion, channel 5
    ADC1->SQR1 &= ~(ADC_SQR1_L | ADC_SQR1_SQ1);
    ADC1->SQR1 |=  (5U << ADC_SQR1_SQ1_Pos);

    // sample time for channel 5, not channel 0
    ADC1->SMPR1 &= ~(7U << ADC_SMPR1_SMP5_Pos);
    ADC1->SMPR1 |=  (4U << ADC_SMPR1_SMP5_Pos);

    // ADC DMA + external trigger
    ADC1->CFGR &= ~(
        ADC_CFGR_DMAEN |
        ADC_CFGR_DMACFG |
        ADC_CFGR_EXTEN |
        ADC_CFGR_EXTSEL
    );
    ADC1->CFGR |= ADC_CFGR_DMAEN;
    ADC1->CFGR |= ADC_CFGR_DMACFG;
    ADC1->CFGR |= (1U << ADC_CFGR_EXTEN_Pos);     // rising edge
    ADC1->CFGR |= (0xBU << ADC_CFGR_EXTSEL_Pos);  // TIM2_TRGO

    // DMA
    DMA1_Channel1->CCR = 0;
    DMA1_Channel1->CPAR  = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR  = (uint32_t)adc_buffer;
    DMA1_Channel1->CNDTR = BUFFER_SIZE;

    DMA1_Channel1->CCR |= DMA_CCR_MINC;
    DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel1->CCR |= DMA_CCR_CIRC;

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C1S;

    DMA1_Channel1->CCR |= DMA_CCR_EN;

    // enable ADC
    ADC1->ISR |= ADC_ISR_ADRDY;   // clear ADRDY if needed
    ADC1->CR  |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {}

    // do NOT software-start here if TIM2 is supposed to trigger each sample
    ADC1->CR |= ADC_CR_ADSTART;
}
uint16_t dma_get_write_index(void)
{
    uint16_t remaining = DMA1_Channel1->CNDTR;
    uint16_t idx = BUFFER_SIZE - remaining;

    if (idx >= BUFFER_SIZE) {
        idx = 0;
    }

    return idx;
}
void copy_latest_window(uint16_t *dst, uint16_t end_index)
{
    uint16_t first_len = BUFFER_SIZE - end_index;

    memcpy(dst, (const void *)&adc_buffer[end_index], first_len * sizeof(uint16_t));

    if (end_index > 0) {
        memcpy(&dst[first_len], (const void *)&adc_buffer[0], end_index * sizeof(uint16_t));
    }
}
