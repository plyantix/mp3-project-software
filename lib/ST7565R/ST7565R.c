#include <string.h>
#include "pico/stdlib.h"

#include "ST7565R.h"

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "font8x8_basic.h"


// 1 bit, 0=1/9, 1=1/7
static const uint8_t cmd_bias_select = 0b10100010;
// 1 bit, 0=column addresses go from left to right, 1=column adresses go from right to left
static const uint8_t cmd_seg_direction = 0b10100000;
// 1 bit, param is bit 3, 0=row addresses go from top to bottom, 1=row adresses go from bottom to top
static const uint8_t cmd_com_direction = 0b11000000;
//                                             ^parameter
// 3 bits
static const uint8_t cmd_regulation_ratio = 0b00100000;
// 2 byte command, 5 bits
static const uint8_t cmd_set_ev_1 = 0b10000001;
static const uint8_t cmd_set_ev_2 = 0b00000000;
// 3 bits, booster, regulator, follower
static const uint8_t cmd_power_control = 0b00101000;
// 1 bit, 1=on, 0=off
static const uint8_t cmd_display_enable = 0b10101110;
// 4 bits
static const uint8_t cmd_set_page_address = 0b10110000;
// 4 bits
static const uint8_t cmd_set_column_address_msb = 0b00010000;
// 4 bits
static const uint8_t cmd_set_column_address_lsb = 0b00000000;

uint8_t display[4][128] = {0};

static uint sck, tx, rx, cs, a0, rst;
static spi_inst_t* spi;

static uint8_t updated_x1 = 0;
static uint8_t updated_x2 = WIDTH;
static uint8_t updated_y1 = 0;
static uint8_t updated_y2 = HEIGHT;

void ST7565R_cmd(uint8_t cmd) {
    gpio_put(a0, 0);
    gpio_put(cs, 0);

    spi_write_blocking(spi, &cmd, 1);
    gpio_put(cs, 1);
}

static void set_column_address(uint8_t column) {
    ST7565R_cmd(cmd_set_column_address_msb | (column >> 4));
    ST7565R_cmd(cmd_set_column_address_lsb | (column & 0x0f));
}

void ST7565R_set_pixel(uint8_t x, uint8_t y, bool value) {
    assert(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
    display[y/8][x] = (display[y/8][x] & ~(1<<(y%8))) | (value << (y%8));
}

void ST7565R_ddram_write(uint8_t* data, size_t len) {
    gpio_put(a0, 1);
    gpio_put(cs, 0);

    spi_write_blocking(spi, data, len);

    gpio_put(cs, 1);

}

void ST7565R_refresh_screen_area(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2) {
    // For every page within the area
    for (uint8_t page = y1 / 8; page < y2 / 8; page++) {
        set_column_address(x1);
        ST7565R_ddram_write(&display[page][x1], x2 - x1);
    }
}

void ST7565R_refresh_screen() {
    ST7565R_refresh_screen_area(updated_x1, updated_x2, updated_y1, updated_y2);
}

void ST7565R_write_text(char* text, uint8_t page, uint8_t x) {
    size_t len = strlen(text);

    // overflow protection
    if (x + len*8 >= WIDTH) {
        len = (WIDTH - x - 1) / 8;
    }

    set_column_address(x);

    for (int i = 0; i < len; i++) {
        ST7565R_ddram_write(font8x8_basic[text[i]], 8);
    }
}

void ST7565R_init(spi_inst_t* spi_inst, uint pin_sck, uint pin_tx, uint pin_rx, uint pin_cs, uint pin_a0, uint pin_rst) {
    spi = spi_inst;
    sck = pin_sck;
    tx = pin_tx;
    rx = pin_rx;
    cs = pin_cs;
    a0 = pin_a0;
    rst = pin_rst;
    spi_init(spi, 8*1000*1000);

    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_function(tx, GPIO_FUNC_SPI);
    gpio_set_function(rx, GPIO_FUNC_SPI);
    
    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);

    gpio_init(a0);
    gpio_set_dir(a0, GPIO_OUT);
    gpio_put(a0, 0);

    gpio_init(rst);
    gpio_set_dir(rst, GPIO_OUT);
    gpio_put(rst, 0);
    
    // Startup sequence, datasheet pg. 37

    sleep_us(1005);
    gpio_put(rst, 1);
    sleep_us(5);
    ST7565R_cmd(cmd_display_enable | 0b0);
    ST7565R_cmd(cmd_bias_select | 0b0);
    ST7565R_cmd(cmd_seg_direction | 0b0);
    ST7565R_cmd(cmd_com_direction | (0b0 << 3));
    ST7565R_cmd(cmd_regulation_ratio | 0b001);
    ST7565R_cmd(cmd_set_ev_1);
    ST7565R_cmd(cmd_set_ev_2 | 0b11111);
    ST7565R_cmd(cmd_power_control | 0b111);

    ST7565R_refresh_screen();
    ST7565R_cmd(cmd_display_enable | 0b1);
}

