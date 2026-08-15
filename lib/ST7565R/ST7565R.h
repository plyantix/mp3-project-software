#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/spi.h"

#define WIDTH 128
#define HEIGHT 32

void ST7565R_init(spi_inst_t* spi_inst,
                  uint pin_sck,
                  uint pin_tx,
                  uint pin_rx,
                  uint pin_cs,
                  uint pin_a0,
                  uint pin_rst);

void ST7565R_set_pixel(uint8_t x, uint8_t y, bool value);
void ST7565R_refresh_screen_area(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
void ST7565R_refresh_screen(void);
void ST7565R_write_text(uint8_t page, uint8_t x, char* format, ...);

