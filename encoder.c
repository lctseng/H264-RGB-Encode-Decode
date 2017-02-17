/*****************************************************************************
 * example.c: libx264 API usage example
 *****************************************************************************
 * Copyright (C) 2014-2017 x264 project
 *
 * Authors: Anton Mitrofanov <BugMaster@narod.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#ifdef _WIN32
#include <io.h>       /* _setmode() */
#include <fcntl.h>    /* _O_BINARY */
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <x264.h>

#include <stdbool.h>

#define FAIL_IF_ERROR( cond, ... )\
do\
{\
    if( cond )\
    {\
        fprintf( stderr, __VA_ARGS__ );\
        goto fail;\
    }\
} while( 0 )


int encoder_raw_frame_size = 0;
x264_picture_t encoder_pic;
x264_picture_t encoder_pic_out;
bool encoder_pic_valid = false;
int endoder_i_frame;
x264_nal_t *encoder_nal;
int encoder_i_nal;

x264_t* encoder_h264_encoder = NULL;

void encoder_cleanup(){
  if(encoder_h264_encoder){
    x264_encoder_close( encoder_h264_encoder );
    encoder_h264_encoder = NULL;
  }
  if(encoder_pic_valid){
    x264_picture_clean( &encoder_pic );
    encoder_pic_valid = false;
  }
}

int encoder_init(int width, int height){
    x264_param_t param;

    encoder_raw_frame_size = width * height * 3;
    endoder_i_frame = 0;

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
    if( x264_picture_alloc( &encoder_pic, param.i_csp, param.i_width, param.i_height ) < 0 ){
      printf("Fail to allocate picture buffer\n");
      encoder_cleanup();
      return -1;
    }
    encoder_pic_valid = true;
    // open encoder
    encoder_h264_encoder = x264_encoder_open( &param );
    if( !encoder_h264_encoder ){
      printf("Fail to allocate open encoder\n");
      encoder_cleanup();
      return -1;
    }
    return 0;
}


uint8_t* encoder_get_raw_data_buf(){
  return encoder_pic.img.plane[0];
}



// assume the RGB-raw data already in "encoder_get_raw_data_buf()"
// param p_encoded_buf[out]: points to the buffer if frame emitted, NULL for no frame emitted
// param p_encoded_size[out]: the size of encoded data, 0 when no frame emitted
int encoder_encode(uint8_t** p_encoded_buf, int* p_encoded_size){
  int i_frame_size;
  encoder_pic.i_pts = endoder_i_frame;
  i_frame_size = x264_encoder_encode( encoder_h264_encoder, &encoder_nal, &encoder_i_nal, &encoder_pic, &encoder_pic_out );
  if(i_frame_size < 0){
    printf("Encoding error\n");
    return -1;
  }
  else if (i_frame_size){
    *p_encoded_size = i_frame_size;
    *p_encoded_buf = encoder_nal->p_payload;
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
