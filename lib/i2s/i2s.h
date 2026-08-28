#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"
#include "hardware/dma.h"

#define AUDIO_BUFFER_SIZE 1024*4
#define BITDEPTH 16

struct {
    // The rare pointer volatile. 
    // The interrupt handler changes the address to create a circular buffer, but the contents of the buffer are never modified from the handler.
    void* volatile addr; 
    volatile uint32_t size;
    volatile bool pending;
} typedef audio_request_t;

void i2s_set_sample_rate(uint32_t rate);
audio_request_t* i2s_init(PIO pio, uint data_pin, uint clock_pin, uint32_t rate);
void i2s_pause();
void i2s_play();

// Safely access the members of the audio request object without its data
// being changed by an interrupt in the middle of a read.
static inline void audio_request_access_start() {
    irq_set_enabled(dma_get_irq_num(0), false);
}

static inline void audio_request_access_stop() {
    irq_set_enabled(dma_get_irq_num(0), true);
}