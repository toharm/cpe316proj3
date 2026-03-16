/*
 * fft.h
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#include <stdint.h>



// Initialize tables/window once at startup.
void fft_mag_init(void);

// Compute magnitude spectrum from uint16_t real samples.
// Input:  samples[BUFFER_SIZE]
// Output: mags[BUFFER_SIZE / 2]
//
// mags[0]           = DC bin magnitude
// mags[1]           = 1st frequency bin
// ...
// mags[BUFFER_SIZE/2 - 1] = highest useful bin below Nyquist
void fft_compute_magnitudes_u16(const uint16_t *samples, uint16_t *mags);

// Optional helper: find strongest non-DC bin.
uint16_t fft_find_peak_bin(const uint16_t *mags, uint16_t start_bin);

float fft_refine_peak_bin_parabolic(const uint16_t *mags, uint16_t peak_bin);



#endif /* INC_FFT_H_ */
