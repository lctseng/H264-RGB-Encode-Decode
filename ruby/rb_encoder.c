#include "rb_encoder.h"

// for encoder
static struct {
  int raw_byte_size;
  uint8_t* raw_buffer;
  bool flag_init;
} encoder_data = {.flag_init = false};

VALUE rb_encoder_init(VALUE self, VALUE Qwidth, VALUE Qheight){
  if(!encoder_data.flag_init){
    int width = NUM2INT(Qwidth);
    int height = NUM2INT(Qheight);
    encoder_data.raw_byte_size = width * height * 3;
    if(encoder_init(width, height) < 0){
      return Qfalse;
    }
    else{
      encoder_data.flag_init = true;
      encoder_data.raw_buffer = encoder_get_raw_data_buf();
      return Qtrue;
    }
  }
  else{
    // already initialized
    return Qtrue;
  }
}

VALUE rb_encoder_cleanup(VALUE self){
  if(encoder_data.flag_init){
    encoder_cleanup();
    encoder_data.flag_init = false;
    return Qtrue;
  }
  else{
    return Qfalse;
  }
}

// return: nil of fail
//         empty string for no output, non-empty string for encoded data
VALUE rb_encoder_encode(VALUE self, VALUE rb_raw_data){
  uint8_t* out_buffer;
  int out_size;
  if(!encoder_data.flag_init){
    rb_raise(rb_eRuntimeError, "Encoder not initialized");
  }
  // copy raw data
  // verify size
  if(RSTRING_LEN(rb_raw_data) > encoder_data.raw_byte_size){
    rb_raise(rb_eRuntimeError, "Raw data is too large! Expect: %d", encoder_data.raw_byte_size);
    return Qnil;
  }
  memcpy(encoder_data.raw_buffer, StringValuePtr(rb_raw_data),encoder_data.raw_byte_size);
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

