#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "tests.h"
#include "pins.h"
#include "tusb_config.h"
#include "tusb.h"
#include "ST7565R.h"
#include "Ff.h"

#include "pico/low_power.h"

int main()
{
    stdio_init_all();
    printf("Starting!\n");
    sleep_ms(5000);
    // while (true) {
    //     sleep_ms(10000); // 52mA
    //     low_power_dormant_for_ms(10000, DORMANT_CLOCK_SOURCE_DEFAULT, NULL); // 38.91 mA
    // }

    // ST7565R_init(LCD_SPI, LCD_SCK, LCD_TX, LCD_RX, LCD_CS, LCD_A0, LCD_RST);
    // gpio_init(LCD_BACKLIGHT);
    // gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);
    // gpio_put(LCD_BACKLIGHT, true);
    // stdio_init_all();
    // FATFS fs;
    // f_mount(&fs, "", 1);
    // int err = tusb_init();
    // ST7565R_write_text(0, 0, "%d", err);
    // rom_reset_usb_boot(1<<LCD_BACKLIGHT, 0);
    // printf("Beginning tests!\n");
    // sleep_ms(5000);
    // hello_world();
    // blink_backlight();
    audio();
    // decode();
    // battery_info();
    // switches();
    // screen();
    // sd_card();
    // while (true) {
    //     tud_task();
    // }
}
