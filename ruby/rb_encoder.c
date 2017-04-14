#include "rb_encoder.h"

// for encoder
typedef struct {
  int raw_byte_size;
  uint8_t* raw_buffer;
  bool flag_init;
  H264EncoderData* internal_data;
} RbEncoderData;

VALUE rb_encoder_initialize(VALUE self, VALUE Qwidth, VALUE Qheight){
  // unwrap
  RbEncoderData* encoder_data;
  Data_Get_Struct(self, RbEncoderData, encoder_data);
  // set the internal data 
  int width = NUM2INT(Qwidth);
  int height = NUM2INT(Qheight);
  encoder_data->raw_byte_size = width * height * 3;
  if(encoder_init(&encoder_data->internal_data ,width, height) < 0){
    // internal data cannot be allocated
    return Qfalse;
  }
  else{
    encoder_data->flag_init = true;
    encoder_data->raw_buffer = encoder_get_raw_data_buf(encoder_data->internal_data);
    return Qtrue;
  }
}

VALUE rb_encoder_dispose(VALUE self){
  // unwrap
  RbEncoderData* encoder_data;
  Data_Get_Struct(self, RbEncoderData, encoder_data);

  if(encoder_data->flag_init){
    // free the internal data
    encoder_dispose(encoder_data->internal_data);
    encoder_data->flag_init = false;
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
  // unwrap
  RbEncoderData* encoder_data;
  Data_Get_Struct(self, RbEncoderData, encoder_data);

  if(!encoder_data->flag_init){
    rb_raise(rb_eRuntimeError, "Encoder not initialized");
  }
  // copy raw data
  // verify size
  if(RSTRING_LEN(rb_raw_data) > encoder_data->raw_byte_size){
    rb_raise(rb_eRuntimeError, "Raw data is too large! Expect: %d", encoder_data->raw_byte_size);
    return Qnil;
  }
  memcpy(encoder_data->raw_buffer, StringValuePtr(rb_raw_data),encoder_data->raw_byte_size);
  // encode!
  if(encoder_encode(encoder_data->internal_data,&out_buffer, &out_size) < 0){
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

void rb_encoder_free(void * ptr){
  RbEncoderData* data = (RbEncoderData*)(ptr);
  if(data->flag_init){
    // not release internal at this time!
    // this prevents the pitfall that user does not dispose the instance
    encoder_dispose(data->internal_data);
    data->flag_init = false;
  }
  free(data);
}


VALUE rb_encoder_alloc(VALUE self){
  RbEncoderData* data = (RbEncoderData*)malloc(sizeof(RbEncoderData));
  return Data_Wrap_Struct(self,  NULL, rb_encoder_free, data);
}
