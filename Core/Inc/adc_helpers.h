/*
 * adc_helpers.h
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#ifndef INC_ADC_HELPERS_H_
#define INC_ADC_HELPERS_H_

#define BUFFER_SIZE 4096

#define ADC_SAMPLE_RATE  1000.0f                     /* 1 kHz ADC     */
#define BIN_RES          (ADC_SAMPLE_RATE / (float)N)    /* ~0.98 Hz/bin  */
#define PID_RATE         1.0f                          /* 50 Hz loop    */
#define ADVANCE          ((int)(ADC_SAMPLE_RATE / PID_RATE))  /* 80 samples */
#define PID_DT           (1.0f / PID_RATE)

extern volatile uint16_t adc_buffer[BUFFER_SIZE];

void ADC_Init(void);

#endif /* INC_ADC_HELPERS_H_ */
