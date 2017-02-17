#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"

#define IN_BUFFER_SIZE 65535

uint8_t in_buffer[IN_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]={0};

int main(int argc, char* argv[]){
  struct AVCodec *pCodec;
  struct AVCodecContext *pCodecCtx= NULL;
  struct AVCodecParserContext *pCodecParserCtx=NULL;

  int frame_count;
  struct AVFrame	*pFrame,*pFrameYUV;
  uint8_t *out_buffer;
  uint8_t *cur_ptr;
  int cur_size;

  struct AVPacket packet;
  int ret, got_picture;

  int y_size;

  enum AVCodecID codec_id=AV_CODEC_ID_H264;
  int first_time=1;

  struct SwsContext *img_convert_ctx;

  av_log_set_level(AV_LOG_DEBUG);

  avcodec_register_all();

  pCodec = avcodec_find_decoder(codec_id);
  if (!pCodec) {
    fprintf(stderr,"Codec not found\n");
    return -1;
  }
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if (!pCodecCtx){
    fprintf(stderr,"Could not allocate video codec context\n");
    return -1;
  }

  pCodecParserCtx=av_parser_init(codec_id);
  if (!pCodecParserCtx){
    fprintf(stderr,"Could not allocate video parser context\n");
    return -1;
  }

  if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    fprintf(stderr,"Could not open codec\n");
    return -1;
  }

  pFrame = av_frame_alloc();
  av_init_packet(&packet);

  int count = 0;
  int input_read = 0;
  int pkt_count = 0;
  
  int consumed = 0;

  int last_frame = 0;
  int need_exit = 0;

  while (1) {
    cur_size = fread(in_buffer, 1, IN_BUFFER_SIZE, stdin);
    input_read += cur_size;
    if (cur_size == 0){
      fprintf(stderr,"End of input (%d) bytes\n", input_read);
      if(consumed > 0){
        last_frame = 1;
        need_exit = 1;
      }
    }
    cur_ptr=in_buffer;

    while (cur_size>0 || last_frame){

      int len = av_parser_parse2(
        pCodecParserCtx, pCodecCtx,
        &packet.data, &packet.size,
        cur_ptr , cur_size ,
        AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

      cur_ptr += len;
      cur_size -= len;
      consumed += len;

      if(packet.size==0){
        if(last_frame){
          last_frame = 0;
          need_exit = 1;
        }
        //fprintf(stderr,"No packet formed, consumed: %d\n", consumed);
        continue;
      }


      //Some Info from AVCodecParserContext
      fprintf(stderr,"New packet formed by consumed: %d\n", consumed);
      consumed = 0;
      fprintf(stderr,"Packet %d Size:%6d\t",++pkt_count,packet.size);
      switch(pCodecParserCtx->pict_type){
      case AV_PICTURE_TYPE_I: fprintf(stderr,"Type: I\t");break;
      case AV_PICTURE_TYPE_P: fprintf(stderr,"Type: P\t");break;
      case AV_PICTURE_TYPE_B: fprintf(stderr,"Type: B\t");break;
      default: fprintf(stderr,"Type: Other\t");break;
      }
      fprintf(stderr,"Output Number:%4d\t",pCodecParserCtx->output_picture_number);

      ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
      if (ret < 0) {
        fprintf(stderr,"Decode Error.\n");
        return ret;
      }
      if (got_picture) {
        if(first_time){
          fprintf(stderr,"\nCodec Full Name:%s\n",pCodecCtx->codec->long_name);
          fprintf(stderr,"width:%d\nheight:%d\n\n",pCodecCtx->width,pCodecCtx->height);
          //SwsContext
          img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
            pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL); 

          pFrameYUV=avcodec_alloc_frame();
          out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));
          avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

          y_size=pCodecCtx->width*pCodecCtx->height * 3;

          first_time=0;
        }

        fprintf(stderr,"Succeed to decode %d frame!\n", ++count);
        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
          pFrameYUV->data, pFrameYUV->linesize);

        fwrite(pFrameYUV->data[0],1,y_size,stdout);
      }
    }
    if(need_exit){
      break;
    }

  }
  //Flush Decoder
  packet.size = 0;
  packet.data = NULL;
  while(1){
    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
    fprintf(stderr,"decode result: %d\n", ret);
    if (ret < 0) {
      fprintf(stderr,"Decode Error.\n");
      return ret;
    }
    if (!got_picture){
      fprintf(stderr,"Flush end\n");
      break;
    }
    if (got_picture) {
      fprintf(stderr,"Flush Decoder: Succeed to decode %d frame!\n", ++count);
      sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
        pFrameYUV->data, pFrameYUV->linesize);

      fwrite(pFrameYUV->data[0],1,y_size,stdout);
    }
  }

  sws_freeContext(img_convert_ctx);
  av_parser_close(pCodecParserCtx);

  av_frame_free(&pFrameYUV);
  av_frame_free(&pFrame);
  avcodec_close(pCodecCtx);
  av_free(pCodecCtx);

  return 0;
}

