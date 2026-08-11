#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "tests.h"
#include "pins.h"

int main()
{
    sleep_ms(5000);
    // rom_reset_usb_boot(1<<LCD_BACKLIGHT, 0);
    // stdio_init_all();
    // printf("Beginning tests!\n");
    // sleep_ms(5000);
    // hello_world();
    // blink_backlight();
    // battery_info();
    // switches();
    screen();
    // sd_card();
    // audio();
    // decode();
}
