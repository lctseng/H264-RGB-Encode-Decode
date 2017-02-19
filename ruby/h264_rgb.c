#include "ruby.h"

#include "rb_encoder.h"
#include "rb_decoder.h"

VALUE H264_RGB;
VALUE H264_RGB_Encoder;
VALUE H264_RGB_Decoder;

// The initialization method for this module
void Init_h264_rgb() {
  H264_RGB = rb_define_module("H264_RGB");

  // encoder
  H264_RGB_Encoder = rb_define_module_under(H264_RGB, "Encoder");
  rb_define_module_function(H264_RGB_Encoder, "init", rb_encoder_init, 2);
  rb_define_module_function(H264_RGB_Encoder, "cleanup", rb_encoder_cleanup, 0);
  rb_define_module_function(H264_RGB_Encoder, "encode", rb_encoder_encode, 1);

  // decoder
  H264_RGB_Decoder = rb_define_module_under(H264_RGB, "Decoder");
  rb_define_module_function(H264_RGB_Decoder, "init", rb_decoder_init, 0);
  rb_define_module_function(H264_RGB_Decoder, "cleanup", rb_decoder_cleanup, 0);
  rb_define_module_function(H264_RGB_Decoder, "parse", rb_decoder_parse, 1);
  rb_define_module_function(H264_RGB_Decoder, "flush", rb_decoder_flush, 0);
}

