/*
 * adc_helpers.h
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#ifndef INC_ADC_HELPERS_H_
#define INC_ADC_HELPERS_H_

#include "main.h"


void ADC_Init(void);
uint16_t dma_get_write_index(void);
void copy_latest_window(uint16_t*, uint16_t);



#endif /* INC_ADC_HELPERS_H_ */
