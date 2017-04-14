#include <stdio.h>

#include "lib/encoder.h"

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

  H264EncoderData* encoder_data;

  if(encoder_init(&encoder_data, width, height, false) < 0){
    // no need to cleanup when fail to init
    printf("Fail to init encoder\n");
    return -1;
  }
  // from now on, cleanup is needed

  raw_buffer = encoder_get_raw_data_buf(encoder_data);
  
  // read frame
  while(fread(raw_buffer, 1, raw_byte_size, fp_in) > 0){
    printf("Reading frame: %d\n", ++in_count);
    // encode it
    if(encoder_encode(encoder_data,&out_buffer, &out_size) < 0){
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
  encoder_dispose(encoder_data);
  return 0;
}
