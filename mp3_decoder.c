#include <stdlib.h>
#include <stdio.h>
//#define MINIMP3_ONLY_MP3 // Removes mp1 and mp2 decoders
//#define MINIMP3_ONLY_SIMD // hardware optimizations
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL // Saves flash by disabling mono to stereo transistion within same file (in spec but rare)
//#define MINIMP3_FLOAT_OUTPUT // mp3dec_decode_frame() returns a float
#define MINIMP3_IMPLEMENTATION
#include "lib/minimp3/minimp3.h"
#include "ff.h"

#include "mp3_decoder.h"

static mp3dec_t mp3d;
static mp3dec_frame_info_t frame_info;
static FIL file;
#define FILE_BUFF_SIZE 1024*32
#define MIN_BUFF_BYTES 1024*16
static uint8_t file_buff[FILE_BUFF_SIZE];
static uint8_t* read_ptr = file_buff;
static uint32_t bytes_remaining = sizeof(file_buff);

static void fill_buffer(uint8_t* from, unsigned int* remaining) {
    if (remaining) {
        *remaining = 0;
    }
    volatile int err = f_read(&file, from, FILE_BUFF_SIZE - (from - file_buff), remaining);
}

void mp3_decoder_init() {
    mp3dec_init(&mp3d);
}

void mp3_open_file(char* path) {
    FRESULT err;
    err = f_open(&file, path, FA_READ);
    printf("f_open error: %d\n", err);
    fill_buffer(file_buff, NULL);
}

// buffer must be able to fit 1152*2 samples
// Returns number of samples written
int mp3_read_samples(mp3d_sample_t* sample_buff) {
    int num_samples = mp3dec_decode_frame(&mp3d, read_ptr, bytes_remaining, sample_buff, &frame_info);
    // printf("%d %d %d %d %d %d\n", frame_info.bitrate_kbps, frame_info.channels, frame_info.frame_bytes);
    read_ptr += frame_info.frame_bytes;
    bytes_remaining -= frame_info.frame_bytes;
    if (bytes_remaining < 1024*16) {
        // printf(".\n");
        // Minimp3 reccomends having at least 16kB of data in the buffer
        uint32_t valid_bytes = bytes_remaining;
        memmove(file_buff, read_ptr, valid_bytes); // move the remaining data to the start of the buffer
        unsigned int br = 0;
        fill_buffer(file_buff + valid_bytes, &br);
        if (br == 0 && valid_bytes == 0) {
            return -1;
        }
        read_ptr = file_buff;
        bytes_remaining = valid_bytes + br;
    }
    if (frame_info.hz != 0) {
        volatile int x = 0;
    }
    return num_samples * (frame_info.channels > 0 ? frame_info.channels : 1);
}
