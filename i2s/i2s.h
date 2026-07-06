#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

#define AUDIO_BUFFER_SIZE 2000
#define BITDEPTH 16

typedef void(*i2s_callback_t)(void *addr, uint32_t len);

void i2s_set_sample_rate(uint32_t rate);
void i2s_init(PIO pio, uint data_pin, uint clock_pin, i2s_callback_t callback, uint32_t rate);
