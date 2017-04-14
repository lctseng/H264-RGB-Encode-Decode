#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"

typedef void (*handler_on_frame_ready)(uint8_t* frame_buf, int frame_size);

typedef struct {
  struct AVCodec *pCodec;
  struct AVCodecContext *pCodecCtx;
  struct AVCodecParserContext *pCodecParserCtx;
  struct AVFrame *pFrame,*pFrameOutput;

  uint8_t *out_buffer;

  struct AVPacket packet;

  enum AVCodecID codec_id;
  struct SwsContext *img_convert_ctx;

  int first_time;
  int width;
  int height;
  int output_size;

  handler_on_frame_ready frame_handler;
  
} H264DecoderData;


int decoder_init(H264DecoderData** p_decoder_data);
void decoder_dispose(H264DecoderData* decoder_data);

handler_on_frame_ready decoder_set_frame_ready_handler(H264DecoderData* decoder_data,handler_on_frame_ready hnd);

int decoder_parse(H264DecoderData* decoder_data,uint8_t* in_buffer, int cur_size);
int decoder_flush(H264DecoderData* decoder_data);

int decoder_input_buffer_padding_size();
