/*
 * fft.h
 *
 *  Created on: Mar 15, 2026
 *      Author: tohar
 *  Implementation logic/algorithm credit to UWash
 */

#ifndef SRC_FFT_H_
#define SRC_FFT_H_

#include<math.h>
#include<stdio.h>
#include<stdint.h>

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

#define q 12 // 4096 amplitudes
#define N (1<<q)

void fft(float*, float*, uint32_t, float*);

#endif /* SRC_FFT_H_ */
