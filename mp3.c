#include <stdio.h>
#include "pico/stdlib.h"

#include "hw_config.h"
#include "ff.h"

#include "tusb_config.h"
#include "tusb.h"

#include "ili9341.h"
#include "gfx.h"

#include "i2s.h"

int main()
{
    stdio_init_all();

    LCD_setPins(11, 13, 10, 14, 15);
    LCD_setSPIperiph(spi1);

    LCD_initDisplay();
    LCD_setRotation(0);
    int c = 0;

    GFX_clearScreen();
    GFX_setCursor(0, 0);
    GFX_printf("LCD Initilized\n");
    
    int err;
    err = tusb_init();
    GFX_printf("TinyUSB Initialized (Error: %d)\n", err);


    FATFS fs;
    FIL file;
    FRESULT res;
    char buff[1000];

    // sleep_ms(5*1000);
    GFX_printf("Attempting to mount file system.\n");


    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        GFX_printf("Could not mount file system! Error: %d\n", res);
        return 0;
    }
    GFX_printf("Sucessfully mounted file system!\n");

    while (true) {
        tud_task(); // TinyUSB
    }
}
