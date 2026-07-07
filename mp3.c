#include <stdio.h>
#include "pico/stdlib.h"

#include "hw_config.h"
#include "ff.h"

#include "tusb_config.h"
#include "tusb.h"

// #include "ili9341.h"
// #include "gfx.h"
#define GFX_printf(...) (void) 0

#include "i2s.h"

#include "music_file.h"

#include "pico/multicore.h"

#define USE_TINYUSB

#define MF_BUFF_SIZE 1024
char mf_buff[MF_BUFF_SIZE]; // Buffer used internally by the mf library
music_file mf = {
    .init = false
};

uint32_t sample_rate = 0;

void i2s_callback(void* buff, uint32_t len) {
    uint32_t written;
    // if the music file is mono, this will not duplicate the samples!!!
    musicFileRead(&mf, buff, len / 2, &written);
    // should also check if written < len/2
}

void load_file(char* path) {
    // later can be upgraded to detect the file type and switch to different decoders
    // The musicFile decoder support mp3 or wav
    if (mf.init) {
        musicFileClose(&mf);
    }
    musicFileCreate(&mf, path, mf_buff, sizeof(mf_buff));
    
    if (sample_rate != mf.sample_rate) {
        i2s_set_sample_rate(mf.sample_rate);
    }
    if (mf.channels != 2) {
        // should probably handle this
    }

}

void core1_main() {
    printf("loading mp3\n");
    load_file("stereo-test.mp3");
    printf("initializing i2s\n");
    i2s_init(pio0, 7, 8, i2s_callback, 44100);
    
    while (true) {
        tight_loop_contents();
    }
}

int main()
{
    stdio_init_all();

    // LCD_setPins(11, 13, 10, 14, 15);
    // LCD_setSPIperiph(spi1);

    // LCD_initDisplay();
    // LCD_setRotation(0);
    // int c = 0;

    // GFX_clearScreen();
    // GFX_setCursor(0, 0);
    // GFX_printf("LCD Initilized\n");

#ifdef USE_TINYUSB
    int err;
    err = tusb_init();
    GFX_printf("TinyUSB Initialized (Error: %d)\n", err);
#endif

    FATFS fs;
    FIL file;
    FRESULT res;
    char buff[1000];

    sleep_ms(5*1000);
    printf("Attempting to mount file system.\n");


    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount file system! Error: %d\n", res);
        return 0;
    }
    printf("Sucessfully mounted file system!\n");

    // core1_main();

    while (true) {
#ifdef USE_TINYUSB
        tud_task(); // TinyUSB
#else 
        tight_loop_contents();
#endif
    }
}
