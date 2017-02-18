#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x264.h>

#include <stdbool.h>


void encoder_cleanup();
int encoder_init(int width, int height);

uint8_t* encoder_get_raw_data_buf();
int encoder_encode(uint8_t** p_encoded_buf, int* p_encoded_size);
