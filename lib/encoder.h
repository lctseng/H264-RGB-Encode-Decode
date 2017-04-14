#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x264.h>

#include <stdbool.h>

typedef struct{
  int raw_frame_size;
  x264_picture_t pic;
  x264_picture_t pic_out;
  bool pic_valid;
  int i_frame;
  x264_nal_t *nal;
  int i_nal;

  x264_t* h264_encoder;
} H264EncoderData;

// free the context that encoder_data points with
void encoder_dispose(H264EncoderData* encoder_data);
// create a new context and let p_encoder_data point to that
int encoder_init(H264EncoderData** p_encoder_data,int width, int height, bool lossless);

uint8_t* encoder_get_raw_data_buf(H264EncoderData* encoder_data);
int encoder_encode(H264EncoderData* encoder_data , uint8_t** p_encoded_buf, int* p_encoded_size);
