#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"

#include "ST7565R.h"

#include "hw_config.h"
#include "ff.h"

#include "i2s.h"

#include "music_file.h"

#include "mp3_decoder.h"

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


#define CALIBRATION (4.057f/4.05f)
void battery_info() {
    ST7565R_init(LCD_SPI, LCD_SCK, LCD_TX, LCD_RX, LCD_CS, LCD_A0, LCD_RST);
    gpio_init(LCD_BACKLIGHT);
    gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);

    gpio_put(LCD_BACKLIGHT, true);
    gpio_init(BATTERY_STAT);
    gpio_set_dir(BATTERY_STAT, GPIO_IN);
    gpio_pull_up(BATTERY_STAT);
    adc_init();
    adc_gpio_init(BATTERY_MONITOR);
    adc_select_input(BATTERY_MONITOR - 26);

    while(true) {
        bool charging = !gpio_get(BATTERY_STAT);
        uint16_t level = adc_read();
        ST7565R_write_text(0, 0, charging ? "Charging!      \n" : "Not charging!\n");
        ST7565R_write_text(1, 0, "Level: %d\n", level);
        ST7565R_write_text(2, 0, "%f V\n", ((float) level / (float) 0x0fff) * 3.3f * 4.2f / 3.f * CALIBRATION);
        sleep_ms(50);
    }
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

    ST7565R_init(LCD_SPI, LCD_SCK, LCD_TX, LCD_RX, LCD_CS, LCD_A0, LCD_RST);
    gpio_init(LCD_BACKLIGHT);
    gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);
    gpio_put(LCD_BACKLIGHT, true);

    while (true) {
        bool sw2 = gpio_get(SW_2);
        bool sw3 = gpio_get(SW_3);
        bool sw4 = gpio_get(SW_4);
        bool sw5 = gpio_get(SW_5);
        bool sw6 = gpio_get(SW_6);

        ST7565R_write_text(0, 0, "2 3 4 5 6");
        ST7565R_write_text(1, 0, "%d %d %d %d %d", sw2, sw3, sw4, sw5, sw6);        
        sleep_ms(50);
    }
}

void screen() {
    ST7565R_init(LCD_SPI, LCD_SCK, LCD_TX, LCD_RX, LCD_CS, LCD_A0, LCD_RST);
    gpio_init(LCD_BACKLIGHT);
    gpio_set_dir(LCD_BACKLIGHT, GPIO_OUT);
    gpio_put(LCD_BACKLIGHT, true);

    for (int i = 0; i < 4; i++) {
        ST7565R_write_text(i, 25, "Hello!");
    }
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

#define SAMPLE_RATE 48000
#define FREQUENCY 440
#define WAVE_TABLE_LEN (SAMPLE_RATE/FREQUENCY)
static int16_t wave_table[WAVE_TABLE_LEN];

void gpio_callback(uint gpio, uint32_t events) {
    printf("restarting\n");
    watchdog_enable(1, 1);
    while (1); 
}

void audio() {
    void* request_buff;
    uint32_t remaining;

    // sleep_ms(20);
    // gpio_init(SW_3);
    // gpio_set_irq_enabled_with_callback(SW_3, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // for (int i = 0; i < WAVE_TABLE_LEN; i++) {
    //     wave_table[i] = (int16_t) ((INT16_MAX - 1) * sinf((float) i / WAVE_TABLE_LEN * 2*M_PI));
    // }
    // printf("Generated wave table!\n");

    gpio_init(MUTE);
    gpio_set_dir(MUTE, GPIO_OUT);
    gpio_put(MUTE, true);

    mp3d_sample_t sample_buff[1152*2];
    mp3_decoder_init();
    mp3_open_file("21. Modern Gamer.mp3");

    audio_request_t* audio_request = i2s_init(pio0, AUDIO_DATA, AUDIO_BCK, SAMPLE_RATE);
    printf("Initialized I2S\n");

    // FIL log_file;
    // f_open(&log_file, "log2.bin", FA_CREATE_ALWAYS | FA_WRITE);

    // printf("Playing\n");
    int pos = 0;
    int overflow_bytes = 0;
    uint32_t loop_counter = 0;
    while (true) {
        // sleep_ms(10);
        if ((loop_counter % 2000) == 0) {
#ifdef AUDIO_TRACE
            printf("irq=%lu request=%lu active=%d overflow=%d\n",
                   dma_irq_count,
                   dma_request_count,
                   active_buff,
                   overflow_bytes);
#endif
        }
        loop_counter++;

        if (audio_request->pending) {
            // printf("recieved audio request\n");
            audio_request_access_start();
            audio_request->pending = false;
            request_buff = audio_request->addr;
            remaining = audio_request->size;
            audio_request_access_stop();

            int to_write;
            int num_samples;
            if (overflow_bytes > 0) {
                memmove(request_buff, ((char*)sample_buff) + (sizeof(sample_buff) - overflow_bytes), overflow_bytes);
                request_buff += overflow_bytes;
                remaining -= overflow_bytes;
                overflow_bytes = 0;
            }

            while (remaining > 0) {
                num_samples = mp3_read_samples(sample_buff);
                if (num_samples <= 0) {
                    memset(request_buff, 0, remaining);
                    remaining = 0;
                    break;
                }
                for (int i = 0; i < 1152*2; i++) {
                    sample_buff[i] = sample_buff[i] / 6;
                }

                uint32_t frame_bytes = num_samples * sizeof(mp3d_sample_t);
                to_write = MIN(frame_bytes, remaining);
                memcpy(request_buff, sample_buff, to_write);
                request_buff += to_write;
                remaining -= to_write;

                if (remaining == 0) {
                    overflow_bytes = 0;
                    break;
                }

                if (audio_request->pending) {
                    memset(request_buff, 0, remaining);
                    remaining = 0;
                    overflow_bytes = 0;
                    break;
                }

                if (to_write < frame_bytes) {
                    overflow_bytes = frame_bytes - to_write;
                    memmove(sample_buff, ((char*)sample_buff) + to_write, overflow_bytes);
                    break;
                }
                overflow_bytes = 0;
            }
        }
    }
}
#undef SAMPLE_RATE
#undef FREQUENCY
#undef WAVE_TABLE_LEN

#pragma endregion