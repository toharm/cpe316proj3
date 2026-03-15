/*
 * fft.c
 *
 *  Created on: Mar 15, 2026
 *      Author: tohar
 */

#include "fft.h"

/**
 * credit: https://www.math.wustl.edu/~victor/mfmm/fourier/fft.c
 * only care about cosine
 */
void fft(float* v, int n, float* tmp)
{
	if(n>1) {			/* otherwise, do nothing and return */
		int k,m;    float z, w, *vo, *ve;
		ve = tmp; vo = tmp+n/2;
		for(k=0; k<n/2; k++) {
		  ve[k] = v[2*k];
		  vo[k] = v[2*k+1];
		}
		fft( ve, n/2, v );
		fft( vo, n/2, v );
		for(m=0; m<n/2; m++) {
		  w = cos(2*PI*m/(double)n);
		  z = w.Re*vo[m];
		  v[m] = ve[m] + z;
		  v[m+n/2] = ve[m] - z;
		}
	}
	return;
}

float get_highest_fft(float* v) {
	float highest = v[0];
	for (int i = 0; i < N; i++) {
		if (v[i] > highest) highest = v[i];
	}
	return highest; // to be used in pid
}
