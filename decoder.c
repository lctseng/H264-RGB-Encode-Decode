#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"

#define IN_BUFFER_SIZE 65535

typedef void (*handler_on_frame_ready)(uint8_t* frame_buf, int frame_size);

struct {
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
  
} decoder_data;


void decoder_cleanup(){
  if(decoder_data.img_convert_ctx){
    sws_freeContext(decoder_data.img_convert_ctx);
    decoder_data.img_convert_ctx = NULL;
  }
  if(decoder_data.pCodecParserCtx){
    decoder_data.pCodecParserCtx = NULL;
    av_parser_close(decoder_data.pCodecParserCtx);
  }
  if(decoder_data.pFrameOutput){
    av_frame_free(&decoder_data.pFrameOutput);
    decoder_data.pFrameOutput = NULL;
  }
  if(decoder_data.pFrame){
    av_frame_free(&decoder_data.pFrame);
    decoder_data.pFrame = NULL;
  }
  if(decoder_data.pCodecCtx){
    avcodec_close(decoder_data.pCodecCtx);
    av_free(decoder_data.pCodecCtx);
    decoder_data.pCodecCtx = NULL;
  }
}

int decoder_init(){
  decoder_data.pCodecCtx = NULL;
  decoder_data.pCodecParserCtx=NULL;
  decoder_data.codec_id=AV_CODEC_ID_H264;
  decoder_data.pFrame = NULL;
  decoder_data.pFrameOutput = NULL;
  decoder_data.img_convert_ctx = NULL;
  decoder_data.frame_handler = NULL;
  decoder_data.first_time = 1;

  avcodec_register_all();

  decoder_data.pCodec = avcodec_find_decoder(decoder_data.codec_id);
  if (!decoder_data.pCodec) {
    fprintf(stderr,"Codec not found\n");
    decoder_cleanup();
    return -1;
  }
  decoder_data.pCodecCtx = avcodec_alloc_context3(decoder_data.pCodec);
  if (!decoder_data.pCodecCtx){
    fprintf(stderr,"Could not allocate video codec context\n");
    decoder_cleanup();
    return -1;
  }

  decoder_data.pCodecParserCtx=av_parser_init(decoder_data.codec_id);
  if (!decoder_data.pCodecParserCtx){
    fprintf(stderr,"Could not allocate video parser context\n");
    decoder_cleanup();
    return -1;
  }

  if(decoder_data.pCodec->capabilities & CODEC_CAP_TRUNCATED)
    decoder_data.pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

  if (avcodec_open2(decoder_data.pCodecCtx, decoder_data.pCodec, NULL) < 0) {
    fprintf(stderr,"Could not open codec\n");
    decoder_cleanup();
    return -1;
  }

  decoder_data.pFrame = av_frame_alloc();
  av_init_packet(&decoder_data.packet);

  return 0;
}

// return: number of packets generated during current call to parse
int decoder_parse(uint8_t* in_buffer, int cur_size){
  uint8_t* cur_ptr = in_buffer;
  int frame_formed = 0;
  int ret, got_picture;
  int first_pass = 1;

  while(cur_size > 0 || first_pass){
    int len = av_parser_parse2(
      decoder_data.pCodecParserCtx, decoder_data.pCodecCtx,
      &decoder_data.packet.data, &decoder_data.packet.size,
      cur_ptr , cur_size ,
      AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

    cur_ptr += len;
    cur_size -= len;
    first_pass = 0;

    if(decoder_data.packet.size >0){
      // a packet is ready!
      ret = avcodec_decode_video2(decoder_data.pCodecCtx, decoder_data.pFrame, &got_picture, &decoder_data.packet);
      if (ret < 0) {
        fprintf(stderr,"Decode Error.\n");
        return ret;
      }
      if (got_picture) {
        ++frame_formed;
        if(decoder_data.first_time){
          //SwsContext
          decoder_data.img_convert_ctx = sws_getContext(decoder_data.pCodecCtx->width,
            decoder_data.pCodecCtx->height, decoder_data.pCodecCtx->pix_fmt,decoder_data.pCodecCtx->width,
            decoder_data.pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL); 

          decoder_data.pFrameOutput=avcodec_alloc_frame();
          decoder_data.out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24,
              decoder_data.pCodecCtx->width, decoder_data.pCodecCtx->height));
          avpicture_fill((AVPicture *)decoder_data.pFrameOutput, decoder_data.out_buffer,
            PIX_FMT_RGB24, decoder_data.pCodecCtx->width, decoder_data.pCodecCtx->height);

          decoder_data.width = decoder_data.pCodecCtx->width;
          decoder_data.height = decoder_data.pCodecCtx->height;
          decoder_data.output_size = decoder_data.width * decoder_data.height * 3;

          decoder_data.first_time = 0;
        }

        sws_scale(decoder_data.img_convert_ctx, (const uint8_t* const*)decoder_data.pFrame->data,
          decoder_data.pFrame->linesize, 0, decoder_data.pCodecCtx->height, 
          decoder_data.pFrameOutput->data, decoder_data.pFrameOutput->linesize);

        if(decoder_data.frame_handler){
          decoder_data.frame_handler(decoder_data.pFrameOutput->data[0], decoder_data.output_size);
        }
      }
    }
  }
  return frame_formed;
}

int decoder_flush(){
  int ret, got_picture;
  int frame_formed = 0;
  //Flush Decoder
  decoder_data.packet.size = 0;
  decoder_data.packet.data = NULL;
  while(1){
    ret = avcodec_decode_video2(decoder_data.pCodecCtx, decoder_data.pFrame, &got_picture, &decoder_data.packet);
    if (ret < 0) {
      fprintf(stderr,"Decode Error.\n");
      return ret;
    }
    if (!got_picture){
      fprintf(stderr,"Flush end\n");
      break;
    }
    if (got_picture) {
      ++frame_formed;
      sws_scale(decoder_data.img_convert_ctx, (const uint8_t* const*)decoder_data.pFrame->data,
        decoder_data.pFrame->linesize, 0, decoder_data.pCodecCtx->height, 
        decoder_data.pFrameOutput->data, decoder_data.pFrameOutput->linesize);

      if(decoder_data.frame_handler){
        decoder_data.frame_handler(decoder_data.pFrameOutput->data[0], decoder_data.output_size);
      }
    }
  }
  return frame_formed;
}
  

FILE *fp_in, *fp_out;


void on_frame_ready(uint8_t* frame_buf, int frame_size){
  static int count = 0;
  printf("Frame Ready: %d\n", ++count);
  fwrite(frame_buf, 1, frame_size, fp_out);
}

uint8_t in_buffer[IN_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]={0};

int main(int argc, char* argv[]){
  int cur_size;

  if(argc < 3){
    printf("Usage: %s [input.h264] [output.rgb]\n", argv[0]);
    return -1;
  }

  // set input/output file
  fp_in = fopen(argv[1], "rb");
  if(!fp_in){
    printf("Cannot open input file\n");
    return -1;
  }
  fp_out = fopen(argv[2], "wb");
  if(!fp_out){
    printf("Cannot open output file\n");
    return -1;
  }

  if(decoder_init() < 0){
    printf("Fail to init decoder\n");
  }

  decoder_data.frame_handler = on_frame_ready;
  
  while(( cur_size = fread(in_buffer, 1, IN_BUFFER_SIZE, fp_in)) > 0){
    decoder_parse(in_buffer, cur_size);
  }
  // flush parser
  decoder_parse(NULL, 0);
  decoder_flush();

  decoder_cleanup();
  fclose(fp_in);
  fclose(fp_out);
  return 0;
}

