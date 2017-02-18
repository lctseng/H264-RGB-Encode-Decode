#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"

typedef void (*handler_on_frame_ready)(uint8_t* frame_buf, int frame_size);


int decoder_init();
void decoder_cleanup();

handler_on_frame_ready decoder_set_frame_ready_handler(handler_on_frame_ready hnd);

int decoder_parse(uint8_t* in_buffer, int cur_size);
int decoder_flush();

int decoder_input_buffer_padding_size();
