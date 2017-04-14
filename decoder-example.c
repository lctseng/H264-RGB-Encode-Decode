#include <stdio.h>
#include "lib/decoder.h"

#define IN_BUFFER_SIZE 65535

FILE *fp_in, *fp_out;

void on_frame_ready(H264DecoderData* decoder_data, uint8_t* frame_buf, int frame_size){
  static int count = 0;
  printf("Frame Ready: %d\n", ++count);
  fwrite(frame_buf, 1, frame_size, fp_out);
}


int main(int argc, char* argv[]){
  uint8_t* in_buffer;
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

  H264DecoderData* decoder_data;

  if(decoder_init(&decoder_data) < 0){
    printf("Fail to init decoder\n");
  }

  decoder_set_frame_ready_handler(decoder_data, on_frame_ready);
  in_buffer = malloc(sizeof(uint8_t) * (IN_BUFFER_SIZE + decoder_input_buffer_padding_size()));
  
  while(( cur_size = fread(in_buffer, 1, IN_BUFFER_SIZE, fp_in)) > 0){
    decoder_parse(decoder_data, in_buffer, cur_size);
  }
  // flush parser
  decoder_parse(decoder_data, NULL, 0);
  decoder_flush(decoder_data);

  decoder_dispose(decoder_data);
  fclose(fp_in);
  fclose(fp_out);
  free(in_buffer);
  return 0;
}

