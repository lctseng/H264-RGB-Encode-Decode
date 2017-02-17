#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x264.h>

#include <stdbool.h>

struct{
  int raw_frame_size;
  x264_picture_t pic;
  x264_picture_t pic_out;
  bool pic_valid;
  int i_frame;
  x264_nal_t *nal;
  int i_nal;

  x264_t* h264_encoder;
} encoder_data;


void encoder_cleanup(){
  if(encoder_data.h264_encoder){
    x264_encoder_close( encoder_data.h264_encoder );
    encoder_data.h264_encoder = NULL;
  }
  if(encoder_data.pic_valid){
    x264_picture_clean( &encoder_data.pic );
    encoder_data.pic_valid = false;
  }
}

int encoder_init(int width, int height){
    x264_param_t param;

    encoder_data.raw_frame_size = width * height * 3;
    encoder_data.i_frame = 0;
    encoder_data.h264_encoder = NULL;
    encoder_data.pic_valid = true;

    /* Get default params for preset/tuning */
    x264_param_default_preset( &param, NULL, "zerolatency" );
    /* Configure non-default params */
    // param.i_csp = X264_CSP_I420;
    param.i_csp = X264_CSP_RGB;
    param.i_width  = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    param.rc.i_qp_constant = 0;
    param.rc.i_rc_method = X264_RC_CQP;
    // alloc picture
    if( x264_picture_alloc( &encoder_data.pic, param.i_csp, param.i_width, param.i_height ) < 0 ){
      printf("Fail to allocate picture buffer\n");
      encoder_cleanup();
      return -1;
    }
    // open encoder
    encoder_data.h264_encoder = x264_encoder_open( &param );
    if( !encoder_data.h264_encoder ){
      printf("Fail to allocate open encoder\n");
      encoder_cleanup();
      return -1;
    }
    return 0;
}


uint8_t* encoder_get_raw_data_buf(){
  return encoder_data.pic.img.plane[0];
}



// assume the RGB-raw data already in "encoder_get_raw_data_buf()"
// param p_encoded_buf[out]: points to the buffer if frame emitted, NULL for no frame emitted
// param p_encoded_size[out]: the size of encoded data, 0 when no frame emitted
int encoder_encode(uint8_t** p_encoded_buf, int* p_encoded_size){
  int i_frame_size;
  encoder_data.pic.i_pts = encoder_data.i_frame;
  i_frame_size = x264_encoder_encode( encoder_data.h264_encoder, &encoder_data.nal, &encoder_data.i_nal, &encoder_data.pic, &encoder_data.pic_out );
  if(i_frame_size < 0){
    printf("Encoding error\n");
    return -1;
  }
  else if (i_frame_size){
    *p_encoded_size = i_frame_size;
    *p_encoded_buf = encoder_data.nal->p_payload;
  }
  else{
    *p_encoded_size = 0;
    *p_encoded_buf = NULL;
  }
  return 0;
}

int main( int argc, char **argv ){
  int width , height;
  uint8_t* raw_buffer;
  uint8_t* out_buffer;
  int out_size;
  FILE *fp_in, *fp_out;
  int raw_byte_size;

  int in_count = 0;
  int out_count = 0;

  if(argc < 5){
    printf("Usage: %s [width] [height] [input.rgb] [output.h264]\n", argv[0]);
    return -1;
  }
  
  // set width and height
  width = atoi(argv[1]);
  height = atoi(argv[2]);
  raw_byte_size = width * height * 3;

  // set input/output file
  fp_in = fopen(argv[3], "rb");
  if(!fp_in){
    printf("Cannot open input file\n");
    return -1;
  }
  fp_out = fopen(argv[4], "wb");
  if(!fp_out){
    printf("Cannot open output file\n");
    return -1;
  }

  if(encoder_init(width, height) < 0){
    // no need to cleanup when fail to init
    printf("Fail to init encoder\n");
    return -1;
  }
  // from now on, cleanup is needed

  raw_buffer = encoder_get_raw_data_buf();
  
  // read frame
  while(fread(raw_buffer, 1, raw_byte_size, fp_in) > 0){
    printf("Reading frame: %d\n", ++in_count);
    // encode it
    if(encoder_encode(&out_buffer, &out_size) < 0){
      printf("Encode error\n");
      goto fail;
    }
    // check if there is output frame
    // but we should always have output
    if(out_size > 0){
      printf("Writing frame: %d\n", ++out_count);
      fwrite(out_buffer, 1, out_size, fp_out);
    }
  }
  // There MUST be no delayed frames
  
fail:
  fclose(fp_in);
  fclose(fp_out);
  encoder_cleanup();
  return 0;
}
