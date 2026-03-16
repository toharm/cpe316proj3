/*
 * fft_schedule.c
 *
 *  Created on: Mar 15, 2026
 *      Author: alexi
 */

#include "fft_scheduler.h"
#include "adc_helpers.h"
#include "main.h"

static uint16_t samples_since_fft = 0;
static uint16_t last_dma_index = 0;
static uint32_t total_samples_seen = 0;

static volatile uint8_t fft_pending = 0;
static volatile uint16_t fft_end_index = 0;

void fft_scheduler_init(void)
{
    last_dma_index = dma_get_write_index();
    samples_since_fft = 0;
    total_samples_seen = 0;
    fft_pending = 0;
    fft_end_index = 0;
}

void fft_scheduler_poll(void)
{
    uint16_t cur = dma_get_write_index();
    uint16_t advanced;

    if (cur >= last_dma_index) {
        advanced = cur - last_dma_index;
    } else {
        advanced = (BUFFER_SIZE - last_dma_index) + cur;
    }

    last_dma_index = cur;
    samples_since_fft += advanced;
    total_samples_seen += advanced;

    if ((samples_since_fft >= BUFFER_SIZE) &&
        (samples_since_fft >= HOP_SIZE) &&
        !fft_pending) {
        fft_end_index = cur;
        fft_pending = 1;
        samples_since_fft -= HOP_SIZE;
    }
}

uint8_t fft_scheduler_take_pending_end_index(uint16_t *end_index)
{
    uint8_t had_pending = 0;

    __disable_irq();
    if (fft_pending) {
        *end_index = fft_end_index;
        fft_pending = 0;
        had_pending = 1;
    }
    __enable_irq();

    return had_pending;
}
