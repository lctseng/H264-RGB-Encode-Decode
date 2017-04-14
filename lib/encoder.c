#include "encoder.h"



void encoder_dispose(H264EncoderData* encoder_data){
  if(encoder_data->h264_encoder){
    x264_encoder_close( encoder_data->h264_encoder );
    encoder_data->h264_encoder = NULL;
  }
  if(encoder_data->pic_valid){
    x264_picture_clean( &encoder_data->pic );
    encoder_data->pic_valid = false;
  }
  // free the data
  free(encoder_data);
}

int encoder_init(H264EncoderData** p_encoder_data, int width, int height){
  
  // malloc new context
  H264EncoderData* encoder_data;
  encoder_data = (H264EncoderData*)malloc(sizeof(H264EncoderData));


  x264_param_t param;

  encoder_data->raw_frame_size = width * height * 3;
  encoder_data->i_frame = 0;
  encoder_data->h264_encoder = NULL;
  encoder_data->pic_valid = true;

  /* Get default params for preset/tuning */
  x264_param_default_preset( &param, NULL, "zerolatency" );
  /* Configure non-default params */
  param.i_csp = X264_CSP_RGB;
  param.i_width  = width;
  param.i_height = height;
  param.b_vfr_input = 0;
  param.b_repeat_headers = 1;
  param.b_annexb = 1;
  param.rc.i_qp_constant = 0;
  param.rc.i_rc_method = X264_RC_CQP;
  // alloc picture
  if( x264_picture_alloc( &encoder_data->pic, param.i_csp, param.i_width, param.i_height ) < 0 ){
    printf("Fail to allocate picture buffer\n");
    encoder_dispose(encoder_data);
    return -1;
  }
  // open encoder
  encoder_data->h264_encoder = x264_encoder_open( &param );
  if( !encoder_data->h264_encoder ){
    printf("Fail to allocate open encoder\n");
    encoder_dispose(encoder_data);
    return -1;
  }
  // set the pointer to new encoder data
  *p_encoder_data = encoder_data;
  return 0;
}


uint8_t* encoder_get_raw_data_buf(H264EncoderData* encoder_data){
  return encoder_data->pic.img.plane[0];
}



// assume the RGB-raw data already in "encoder_get_raw_data_buf()"
// param p_encoded_buf[out]: points to the buffer if frame emitted, NULL for no frame emitted
// param p_encoded_size[out]: the size of encoded data, 0 when no frame emitted
int encoder_encode(H264EncoderData* encoder_data, uint8_t** p_encoded_buf, int* p_encoded_size){
  int i_frame_size;
  encoder_data->pic.i_pts = encoder_data->i_frame;
  i_frame_size = x264_encoder_encode( encoder_data->h264_encoder, &encoder_data->nal, &encoder_data->i_nal, &encoder_data->pic, &encoder_data->pic_out );
  if(i_frame_size < 0){
    printf("Encoding error\n");
    return -1;
  }
  else if (i_frame_size){
    *p_encoded_size = i_frame_size;
    *p_encoded_buf = encoder_data->nal->p_payload;
  }
  else{
    *p_encoded_size = 0;
    *p_encoded_buf = NULL;
  }
  return 0;
}

