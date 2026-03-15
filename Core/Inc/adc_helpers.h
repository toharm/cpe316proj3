/*
 * adc_helpers.h
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#ifndef INC_ADC_HELPERS_H_
#define INC_ADC_HELPERS_H_

#define BUFFER_SIZE 512


extern volatile uint16_t adc_buffer[BUFFER_SIZE];

void ADC_Init(void);


#endif /* INC_ADC_HELPERS_H_ */
