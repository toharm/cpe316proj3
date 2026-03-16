#include "fft.h"
#include <math.h>
#include <string.h>
#include "main.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if (BUFFER_SIZE & (BUFFER_SIZE - 1)) != 0
#error "BUFFER_SIZE must be a power of 2"
#endif

typedef struct {
    float real;
    float imag;
} complex_t;

static complex_t fft_buf[BUFFER_SIZE];
static float hann_window[BUFFER_SIZE];
static uint8_t fft_initialized = 0;

volatile uint16_t last_dma_index = 0;


static complex_t c_add(complex_t a, complex_t b) {
    complex_t r = {a.real + b.real, a.imag + b.imag};
    return r;
}

static complex_t c_sub(complex_t a, complex_t b) {
    complex_t r = {a.real - b.real, a.imag - b.imag};
    return r;
}

static complex_t c_mul(complex_t a, complex_t b) {
    complex_t r = {
        a.real * b.real - a.imag * b.imag,
        a.real * b.imag + a.imag * b.real
    };
    return r;
}

static void bit_reverse(complex_t *x, int n) {
    int i, j = 0, k;

    for (i = 1; i < n; i++) {
        k = n >> 1;
        while (j & k) {
            j ^= k;
            k >>= 1;
        }
        j ^= k;

        if (i < j) {
            complex_t tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }
}

static void fft_inplace(complex_t *x, int n) {
    bit_reverse(x, n);

    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * (float)M_PI / (float)len;
        complex_t wlen = {cosf(angle), sinf(angle)};

        for (int i = 0; i < n; i += len) {
            complex_t w = {1.0f, 0.0f};

            for (int j = 0; j < len / 2; j++) {
                complex_t u = x[i + j];
                complex_t v = c_mul(x[i + j + len / 2], w);

                x[i + j] = c_add(u, v);
                x[i + j + len / 2] = c_sub(u, v);

                w = c_mul(w, wlen);
            }
        }
    }
}

void fft_mag_init(void) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        hann_window[i] = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (BUFFER_SIZE - 1)));
    }
    fft_initialized = 1;
}

void fft_compute_magnitudes_u16(const uint16_t *samples, uint16_t *mags) {
    if (!fft_initialized) {
        fft_mag_init();
    }

    // 1) Estimate DC offset
    float mean = 0.0f;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        mean += (float)samples[i];
    }
    mean /= (float)BUFFER_SIZE;

    // 2) Convert to float, remove DC, apply Hann window
    for (int i = 0; i < BUFFER_SIZE; i++) {
        float x = ((float)samples[i] - mean) * hann_window[i];
        fft_buf[i].real = x;
        fft_buf[i].imag = 0.0f;
    }

    // 3) FFT
    fft_inplace(fft_buf, BUFFER_SIZE);

    // 4) Magnitudes, scaled to uint16_t
    //
    // We only keep bins [0, BUFFER_SIZE/2 - 1].
    // sqrtf() is used because you asked for magnitudes, not power.
    //
    // Scaling note:
    // Raw FFT magnitudes can be much larger than input sample values.
    // Here we normalize roughly by BUFFER_SIZE so output fits more naturally
    // into uint16_t. You can tune this scale for your application.
    for (int k = 0; k < BUFFER_SIZE / 2; k++) {
        float re = fft_buf[k].real;
        float im = fft_buf[k].imag;
        float mag = sqrtf(re * re + im * im);

        // Basic normalization
        mag /= (float)BUFFER_SIZE;

        // Optional extra scaling for nicer uint16_t usage.
        // Tune this number based on your ADC range and desired output.
        mag *= 2.0f;

        if (mag < 0.0f) {
            mag = 0.0f;
        }
        if (mag > 65535.0f) {
            mag = 65535.0f;
        }

        mags[k] = (uint16_t)(mag + 0.5f);
    }
}

uint16_t fft_find_peak_bin(const uint16_t *mags, uint16_t start_bin) {
    uint16_t max_bin = start_bin;
    uint16_t max_val = mags[start_bin];

    for (uint16_t k = start_bin + 1; k < (BUFFER_SIZE / 2); k++) {
        if (mags[k] > max_val) {
            max_val = mags[k];
            max_bin = k;
        }
    }

    return max_bin;
}

float fft_refine_peak_bin_parabolic(const uint16_t *mags, uint16_t peak_bin)
{
    /* Can't interpolate at the edges */
    if (peak_bin == 0 || peak_bin >= (BUFFER_SIZE / 2 - 1)) {
        return (float)peak_bin;
    }

    float left   = (float)mags[peak_bin - 1];
    float center = (float)mags[peak_bin];
    float right  = (float)mags[peak_bin + 1];

    float denom = (left - 2.0f * center + right);

    /* Avoid divide-by-zero or nearly flat cases */
    if (denom > -1e-12f && denom < 1e-12f) {
        return (float)peak_bin;
    }

    float delta = 0.5f * (left - right) / denom;

    /* Clamp for safety */
    if (delta > 0.5f) {
        delta = 0.5f;
    } else if (delta < -0.5f) {
        delta = -0.5f;
    }

    return (float)peak_bin + delta;
}
