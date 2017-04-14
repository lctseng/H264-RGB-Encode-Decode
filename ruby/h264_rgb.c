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
  H264_RGB_Encoder = rb_define_class_under(H264_RGB, "Encoder", rb_cObject);
  rb_define_alloc_func(H264_RGB_Encoder, rb_encoder_alloc);
  rb_define_method(H264_RGB_Encoder, "initialize", rb_encoder_initialize, 3);
  rb_define_method(H264_RGB_Encoder, "dispose", rb_encoder_dispose, 0);
  rb_define_method(H264_RGB_Encoder, "encode", rb_encoder_encode, 1);

  // decoder
  H264_RGB_Decoder = rb_define_class_under(H264_RGB, "Decoder", rb_cObject);
  rb_define_alloc_func(H264_RGB_Decoder, rb_decoder_alloc);
  rb_define_method(H264_RGB_Decoder, "initialize", rb_decoder_initialize, 0);
  rb_define_method(H264_RGB_Decoder, "dispose", rb_decoder_dispose, 0);
  rb_define_method(H264_RGB_Decoder, "parse", rb_decoder_parse, 1);
  rb_define_method(H264_RGB_Decoder, "flush", rb_decoder_flush, 0);
}

