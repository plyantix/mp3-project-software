#include <stdio.h>
#include "pico/stdlib.h"

#include "hw_config.h"
#include "ff.h"

#include "tusb_config.h"
#include "tusb.h"

#include "ili9341.h"
#include "gfx.h"

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

    int sector_size, sector_count;
    err = disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);
    GFX_printf("Sector count error: %x, count: %d\n", err, sector_count);

    // err = disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);
    // GFX_printf("Sector size error: %x, size: %d\n", err, sector_size);

    // GFX_printf("Enter text:\n");

    // int index = 0;
    // while (true) {
    //     // Non-blocking read (returns PICO_ERROR_TIMEOUT if no character is sent)
    //     int c = stdio_getchar();
        
    //     if (c != PICO_ERROR_TIMEOUT) {
    //         // Check for line endings
    //         if (c == '\n' || c == '\r') {
    //             buff[index] = '\0'; // Terminate string
    //             if (index > 0) {
    //                 index = 0; // Reset buffer index
    //             }
    //             break;
    //         } else if (index < sizeof(buff) - 1) {
    //             buff[index++] = (char)c; // Store character
    //         }
    //     }
    // }
    // sleep_ms(10);
    // puts_raw(buff);
    
    // f_open(&file, "test.txt", FA_OPEN_APPEND | FA_WRITE);

    // f_puts(buff, &file);

    // f_close(&file);

    // f_open(&file, "test.txt", FA_READ);

    // f_gets(buff, sizeof(buff), &file);

    // printf(buff);
    // FATFS fs;
    // f_mount(&fs, "", 1);
    while (true) {
        tud_task(); // TinyUSB
    }
}
