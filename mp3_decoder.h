#pragma once
#include "lib/minimp3/minimp3.h"

void mp3_decoder_init();
void mp3_open_file(char* path);
int mp3_read_samples(mp3d_sample_t* samples);