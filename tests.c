#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "ST7565R.h"

#include "hw_config.h"
#include "ff.h"

#include "i2s.h"

#include "music_file.h"

#include "pins.h"

#include "tests.h"


void hello_world() {
    while (true) {
        printf("Hello World!\n");
        sleep_ms(500);
    }
}

void blink_backlight() {
    gpio_init(LCD_BACKLIGHT);
    gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);
    while (true) {
        gpio_put(LCD_BACKLIGHT, 1);
        sleep_ms(500);
        gpio_put(LCD_BACKLIGHT, 0);
        sleep_ms(500);
        printf("Hello World!\n");
    }
}

void battery_info() {
    gpio_init(BATTERY_STAT);
    gpio_set_dir(BATTERY_STAT, GPIO_IN);
    gpio_pull_up(BATTERY_STAT);
    adc_init();
    adc_gpio_init(BATTERY_MONITOR);

    bool charging = !gpio_get(BATTERY_STAT);
    uint16_t level = adc_read();

    printf(charging ? "Battery is charging!\n" : "Battery is not charging!\n");
    printf("Raw battery level: %d\n", level);
    printf("Battery voltage: %f V\n", ((float) level / 0x0fff) * 3.3f * 4.2f / 3.f);
}

void switches() {
    gpio_init(SW_2);
    gpio_init(SW_3);
    gpio_init(SW_4);
    gpio_init(SW_5);
    gpio_init(SW_6);
    
    gpio_set_dir(SW_2, GPIO_IN);
    gpio_set_dir(SW_3, GPIO_IN);
    gpio_set_dir(SW_4, GPIO_IN);
    gpio_set_dir(SW_5, GPIO_IN);
    gpio_set_dir(SW_6, GPIO_IN);

    while (true) {
        bool sw2 = gpio_get(SW_2);
        bool sw3 = gpio_get(SW_3);
        bool sw4 = gpio_get(SW_4);
        bool sw5 = gpio_get(SW_5);
        bool sw6 = gpio_get(SW_6);

        printf("2 3 4 5 6\n");
        printf("%d %d %d %d %d %d\n\n", sw2, sw3, sw4, sw5, sw6);        
        sleep_ms(50);
    }
}

void screen() {
    ST7565R_init(LCD_SPI, LCD_SCK, LCD_TX, LCD_RX, LCD_CS, LCD_A0, LCD_RST);
    gpio_init(LCD_BACKLIGHT);
    gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);
    gpio_put(LCD_BACKLIGHT, true);

    ST7565R_write_text("Hello World!", 2, 25);
}

void sd_card() {
    FATFS fs;
    FIL file;
    FRESULT res;

    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount file system! Error: %d\n", res);
        return;
    }
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Could not open file! Error: %d\n", res);
        return;
    }
    char message[] = "Hello World!";
    uint bw;
    res = f_write(&file, message, sizeof(message), &bw);
    if (res != FR_OK) {
        printf("Could not write to file! Error: %d\n", res);
    return;
    }
    printf("Wrote to file!\n");
    f_close(&file);

    char read[13];

    res = f_open(&file, "test.txt", FA_READ);
    if (res != FR_OK) {
        printf("Could not open file! Error: %d\n", res);
        return;
    }
    res = f_read(&file, &read, sizeof(read), &bw);
    if (res != FR_OK) {
        printf("Could not read file! Error: %d\n", res);
    return;
    }
    printf("Read message from file:\n%s\n", read);
}

#pragma region AUDIO_TEST

#define SAMPLE_RATE 44100
#define FREQUENCY 440
#define WAVE_TABLE_LEN (SAMPLE_RATE/FREQUENCY)
static int16_t wave_table[WAVE_TABLE_LEN];

static void callback(void* addr, const uint32_t len) {
    static int pos = 0;

    for (int i = 0; i < len / 4; i++) {
        ((int16_t*) addr)[i*2] = wave_table[pos];
        ((int16_t*) addr)[i*2+1] = wave_table[pos];
        pos++;
        pos %= WAVE_TABLE_LEN;
    }
}

void audio() {
    for (int i = 0; i < WAVE_TABLE_LEN; i++) {
        wave_table[i] = (int16_t) ((INT16_MAX - 1) * sinf((float) i / WAVE_TABLE_LEN * 2*M_PI));
    }
    printf("Generated wave table!\n");

    gpio_init(MUTE);
    gpio_set_dir(MUTE, GPIO_OUT);
    gpio_put(MUTE, true);

    i2s_init(pio0, AUDIO_DATA, AUDIO_BCK, callback, SAMPLE_RATE);
    printf("Initialized I2S\n");
    while (true) {
        tight_loop_contents();
    }
}
#undef SAMPLE_RATE
#undef FREQUENCY
#undef WAVE_TABLE_LEN

void mp3_decode() {

}

#pragma endregion

#pragma region DECODE_TEST
#define MF_BUFF_SIZE 1024 * 32


char mf_buff[MF_BUFF_SIZE]; // Buffer used internally by the mf library
music_file mf = {
    .init = false
};

void decode_callback(void* buff, uint32_t len) {
    uint32_t written = 0;
    // if the music file is mono, this will not duplicate the samples!!!
    while (written < len / 2) {
        musicFileRead(&mf, (uint16_t*) buff, len / 2 - written, &written);
    }
    // printf("Music file written, len: %d, written: %d\n", len, written);
    // should also check if written < len/2
}

FATFS fs;
void decode() {
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount file system! Error: %d\n", res);
        return;
    }
    printf("Sucessfully mounted file system!\n");

    gpio_init(MUTE);
    gpio_set_dir(MUTE, GPIO_OUT);
    gpio_put(MUTE, true);

    
    
    bool err = musicFileCreate(&mf, "21. Modern Gamer.wav", mf_buff, sizeof(mf_buff));
    
    printf("Created music file, error: %d.\n", err);
    
    i2s_init(pio0, AUDIO_DATA, AUDIO_BCK, decode_callback, 48000);
    printf("Initialized i2s!\n");

    while (true) {
        tight_loop_contents();
    }
}

#undef MF_BUFF_SIZE
#pragma endregion