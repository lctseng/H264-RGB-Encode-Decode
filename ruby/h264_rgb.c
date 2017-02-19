#include "stdbool.h"

#include "ruby.h"

#include "lib/encoder.h"
#include "lib/decoder.h"

VALUE H264_RGB;
VALUE H264_RGB_Encoder;
VALUE H264_RGB_Decoder;


int raw_byte_size;
uint8_t* raw_buffer;
bool flag_encoder_init = false;

// Our 'test1' method.. it simply returns a value of '10' for now.
VALUE method_test1(VALUE self) {
  int x = 10;
  return INT2NUM(x);
}

static VALUE rb_encoder_init(VALUE self, VALUE Qwidth, VALUE Qheight){
  int width = NUM2INT(Qwidth);
  int height = NUM2INT(Qheight);
  raw_byte_size = width * height * 3;
  if(encoder_init(width, height) < 0){
    return Qfalse;
  }
  else{
    flag_encoder_init = true;
    raw_buffer = encoder_get_raw_data_buf();
    return Qtrue;
  }
}

static VALUE rb_encoder_cleanup(VALUE self){
  if(flag_encoder_init){
    encoder_cleanup();
    flag_encoder_init = false;
    return Qtrue;
  }
  else{
    return Qfalse;
  }
}

// return: nil of fail
//         empty string for no output, non-empty string for encoded data
static VALUE rb_encoder_encode(VALUE self, VALUE rb_raw_data){
  uint8_t* out_buffer;
  int out_size;
  if(!flag_encoder_init){
    rb_raise(rb_eRuntimeError, "Encode not initialized");
  }
  // copy raw data
  // verify size
  if(RSTRING_LEN(rb_raw_data) > raw_byte_size){
    rb_raise(rb_eRuntimeError, "Raw data is too large! Expect: %d", raw_byte_size);
    return Qnil;
  }
  memcpy(raw_buffer, StringValuePtr(rb_raw_data),raw_byte_size);
  // encode!
  if(encoder_encode(&out_buffer, &out_size) < 0){
    return Qnil;
  }
  // create encoded result
  if(out_size > 0){
    return rb_str_new(out_buffer, out_size);
  }
  else{
    // empty result
    return rb_str_new("", 0);
  }
}

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
}

