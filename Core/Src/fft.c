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
void fft(float* re, float* im, uint32_t n, float* tmp)
{
	if(n>1) {			/* otherwise, do nothing and return */
		uint32_t k,m;
		float w_re, w_im, z_re, z_im;
		float *re_e, *re_o, *im_e, *im_o;

		re_e = tmp;
		re_o = tmp + n/2;

		im_e = tmp + n;
		im_o = tmp + n + n/2;

		for(k=0; k<n/2; k++) {
			re_e[k] = re[2*k];
			re_o[k] = re[2*k+1];
			im_e[k] = im[2*k];
			im_o[k] = im[2*k+1];
		}

		fft(re_e, im_e, n/2, re);
		fft(re_o, im_o, n/2, re);

		for(m=0; m<n/2; m++) {
			w_re = cos(2*PI*m/(double)n);
			w_im = -sin(2*PI*m/(double)n);

			z_re = w_re*re_o[m] - w_im*im_o[m];
			z_im = w_re*im_o[m] + w_im*re_o[m];

			re[m] = re_e[m] + z_re;
			im[m] = im_e[m] + z_im;
			re[m+n/2] = re_e[m] - z_re;
			im[m+n/2] = im_e[m] - z_im;
		}
	}
	return;
}
