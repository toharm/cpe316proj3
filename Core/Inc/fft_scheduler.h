/*
 * fft_scheduler.h
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#ifndef INC_FFT_SCHEDULER_H_
#define INC_FFT_SCHEDULER_H_

#include <stdint.h>

void fft_scheduler_init(void);
void fft_scheduler_poll(void);
uint8_t fft_scheduler_take_pending_end_index(uint16_t *end_index);

#endif /* INC_FFT_SCHEDULER_H_ */
