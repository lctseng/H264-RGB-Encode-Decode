#include "decoder.h"


handler_on_frame_ready decoder_set_frame_ready_handler(H264DecoderData* decoder_data,handler_on_frame_ready hnd){
  handler_on_frame_ready old_hnd = decoder_data->frame_handler;
  decoder_data->frame_handler = hnd;
  return old_hnd;
}

void decoder_dispose(H264DecoderData* decoder_data){
  if(decoder_data->img_convert_ctx){
    sws_freeContext(decoder_data->img_convert_ctx);
  }
  if(decoder_data->pCodecParserCtx){
    av_parser_close(decoder_data->pCodecParserCtx);
  }
  if(decoder_data->pFrameOutput){
    av_frame_free(&decoder_data->pFrameOutput);
  }
  if(decoder_data->pFrame){
    av_frame_free(&decoder_data->pFrame);
  }
  if(decoder_data->pCodecCtx){
    avcodec_close(decoder_data->pCodecCtx);
    av_free(decoder_data->pCodecCtx);
  }
  free(decoder_data);
}

int decoder_init(H264DecoderData** p_decoder_data){
  
  H264DecoderData* decoder_data = (H264DecoderData*)malloc(sizeof(H264DecoderData));

  decoder_data->pCodecCtx = NULL;
  decoder_data->pCodecParserCtx=NULL;
  decoder_data->codec_id=AV_CODEC_ID_H264;
  decoder_data->pFrame = NULL;
  decoder_data->pFrameOutput = NULL;
  decoder_data->img_convert_ctx = NULL;
  decoder_data->frame_handler = NULL;
  decoder_data->first_time = 1;

  avcodec_register_all();

  decoder_data->pCodec = avcodec_find_decoder(decoder_data->codec_id);
  if (!decoder_data->pCodec) {
    fprintf(stderr,"Codec not found\n");
    decoder_dispose(decoder_data);
    return -1;
  }
  decoder_data->pCodecCtx = avcodec_alloc_context3(decoder_data->pCodec);
  if (!decoder_data->pCodecCtx){
    fprintf(stderr,"Could not allocate video codec context\n");
    decoder_dispose(decoder_data);
    return -1;
  }

  decoder_data->pCodecParserCtx=av_parser_init(decoder_data->codec_id);
  if (!decoder_data->pCodecParserCtx){
    fprintf(stderr,"Could not allocate video parser context\n");
    decoder_dispose(decoder_data);
    return -1;
  }

  if(decoder_data->pCodec->capabilities & CODEC_CAP_TRUNCATED)
    decoder_data->pCodecCtx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

  if (avcodec_open2(decoder_data->pCodecCtx, decoder_data->pCodec, NULL) < 0) {
    fprintf(stderr,"Could not open codec\n");
    decoder_dispose(decoder_data);
    return -1;
  }

  decoder_data->pFrame = av_frame_alloc();
  av_init_packet(&decoder_data->packet);

  *p_decoder_data = decoder_data;

  return 0;
}

// return: number of packets generated during current call to parse
int decoder_parse(H264DecoderData* decoder_data, uint8_t* in_buffer, int cur_size){
  uint8_t* cur_ptr = in_buffer;
  int frame_formed = 0;
  int ret, got_picture;
  int first_pass = 1;

  while(cur_size > 0 || first_pass){
    int len = av_parser_parse2(
      decoder_data->pCodecParserCtx, decoder_data->pCodecCtx,
      &decoder_data->packet.data, &decoder_data->packet.size,
      cur_ptr , cur_size ,
      AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

    cur_ptr += len;
    cur_size -= len;
    first_pass = 0;

    if(decoder_data->packet.size >0){
      // a packet is ready!
      ret = avcodec_decode_video2(decoder_data->pCodecCtx, decoder_data->pFrame, &got_picture, &decoder_data->packet);
      if (ret < 0) {
        fprintf(stderr,"Decode Error.\n");
        return ret;
      }
      if (got_picture) {
        ++frame_formed;
        if(decoder_data->first_time){
          //SwsContext
          decoder_data->img_convert_ctx = sws_getContext(decoder_data->pCodecCtx->width,
            decoder_data->pCodecCtx->height, decoder_data->pCodecCtx->pix_fmt,decoder_data->pCodecCtx->width,
            decoder_data->pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL); 

          decoder_data->pFrameOutput=av_frame_alloc();
          decoder_data->out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24,
              decoder_data->pCodecCtx->width, decoder_data->pCodecCtx->height));
          avpicture_fill((AVPicture *)decoder_data->pFrameOutput, decoder_data->out_buffer,
            PIX_FMT_RGB24, decoder_data->pCodecCtx->width, decoder_data->pCodecCtx->height);

          decoder_data->width = decoder_data->pCodecCtx->width;
          decoder_data->height = decoder_data->pCodecCtx->height;
          decoder_data->output_size = decoder_data->width * decoder_data->height * 3;

          decoder_data->first_time = 0;
        }

        sws_scale(decoder_data->img_convert_ctx, (const uint8_t* const*)decoder_data->pFrame->data,
          decoder_data->pFrame->linesize, 0, decoder_data->pCodecCtx->height, 
          decoder_data->pFrameOutput->data, decoder_data->pFrameOutput->linesize);

        if(decoder_data->frame_handler){
          decoder_data->frame_handler(decoder_data, decoder_data->pFrameOutput->data[0], decoder_data->output_size);
        }
      }
    }
  }
  return frame_formed;
}

int decoder_flush(H264DecoderData* decoder_data){
  int ret, got_picture;
  int frame_formed = 0;
  //Flush Decoder
  decoder_data->packet.size = 0;
  decoder_data->packet.data = NULL;
  while(1){
    ret = avcodec_decode_video2(decoder_data->pCodecCtx, decoder_data->pFrame, &got_picture, &decoder_data->packet);
    if (ret < 0) {
      fprintf(stderr,"Decode Error.\n");
      return ret;
    }
    if (!got_picture){
      break;
    }
    if (got_picture) {
      ++frame_formed;
      sws_scale(decoder_data->img_convert_ctx, (const uint8_t* const*)decoder_data->pFrame->data,
        decoder_data->pFrame->linesize, 0, decoder_data->pCodecCtx->height, 
        decoder_data->pFrameOutput->data, decoder_data->pFrameOutput->linesize);

      if(decoder_data->frame_handler){
        decoder_data->frame_handler(decoder_data, decoder_data->pFrameOutput->data[0], decoder_data->output_size);
      }
    }
  }
  return frame_formed;
}

int decoder_input_buffer_padding_size(){
  return FF_INPUT_BUFFER_PADDING_SIZE;
}
