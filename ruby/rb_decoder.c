#include "rb_decoder.h"



typedef struct {
  bool flag_init;
  VALUE output_frames;

  VALUE padding_str;
  H264DecoderData* internal_data;
} RbDecoderData;


void decoder_mark_rb(RbDecoderData* decoder_data){
  rb_gc_mark(decoder_data->output_frames);
  rb_gc_mark(decoder_data->padding_str);
}

void rb_decoder_free(void * ptr){
  RbDecoderData* data = (RbDecoderData*)(ptr);
  if(data->flag_init){
    // not release internal at this time!
    // this prevents the pitfall that user does not dispose the instance
    decoder_dispose(data->internal_data);
    data->flag_init = false;
  }
  free(data);
}

VALUE rb_decoder_alloc(VALUE self){
  RbDecoderData* data = (RbDecoderData*)malloc(sizeof(RbDecoderData));
  data->flag_init = false;
  data->output_frames = Qnil;
  data->padding_str = Qnil;
  return Data_Wrap_Struct(self, decoder_mark_rb, rb_decoder_free, data);
}

static void on_frame_ready(H264DecoderData* internal_decoder_data, uint8_t* frame_buf, int frame_size){
  // fetch RbDecoderData from internal data
  RbDecoderData* decoder_data = (RbDecoderData*)internal_decoder_data->user_data;
  // make the data as string and push into array
  VALUE frame = rb_str_new(frame_buf, frame_size);
  rb_ary_push(decoder_data->output_frames, frame);
}

VALUE rb_decoder_initialize(VALUE self){
  // unwrap
  RbDecoderData* decoder_data;
  Data_Get_Struct(self, RbDecoderData, decoder_data);

  if(decoder_init(&decoder_data->internal_data) < 0){
    // fail
    return Qfalse;
  }
  else{
    // set link to self
    decoder_data->internal_data->user_data = decoder_data;
    // init others
    decoder_data->flag_init = true;
    decoder_set_frame_ready_handler(decoder_data->internal_data, on_frame_ready);
    decoder_data->output_frames = rb_ary_new();
    // create padding str
    int size = decoder_input_buffer_padding_size();
    char* buf = malloc(sizeof(char) * size);
    memset(buf, 0, size);
    decoder_data->padding_str = rb_str_new(buf, size);
    free(buf);
    // return true
    return Qtrue;
  }
}

VALUE rb_decoder_dispose(VALUE self){
  // unwrap
  RbDecoderData* decoder_data;
  Data_Get_Struct(self, RbDecoderData, decoder_data);

  if(decoder_data->flag_init){
    decoder_dispose(decoder_data->internal_data);
    decoder_data->flag_init = false;
    decoder_data->output_frames = Qnil;
    decoder_data->padding_str = Qnil;
    return Qtrue;
  }
  else{
    return Qfalse;
  }
}

VALUE rb_decoder_parse(VALUE self, VALUE in_buffer){
  // unwrap
  RbDecoderData* decoder_data;
  Data_Get_Struct(self, RbDecoderData, decoder_data);

  if(!decoder_data->flag_init){
    rb_raise(rb_eRuntimeError, "Decoder not initialized");
  }
  int data_size = RSTRING_LEN(in_buffer);
  // ensure the buffer size > padding size
  // if not, enlarge it
  if(data_size < decoder_input_buffer_padding_size()){
    rb_str_buf_append(in_buffer, decoder_data->padding_str);
  }
  // clean up output frames
  rb_ary_clear(decoder_data->output_frames);
  // inject data
  decoder_parse(decoder_data->internal_data,StringValuePtr(in_buffer),data_size);
  // return frames array
  return decoder_data->output_frames;
}
VALUE rb_decoder_flush(VALUE self){
  // unwrap
  RbDecoderData* decoder_data;
  Data_Get_Struct(self, RbDecoderData, decoder_data);

  if(!decoder_data->flag_init){
    rb_raise(rb_eRuntimeError, "Decoder not initialized");
  }
  // clean up output frames
  rb_ary_clear(decoder_data->output_frames);
  decoder_parse(decoder_data->internal_data, NULL, 0);
  decoder_flush(decoder_data->internal_data);
  // return frames array
  return decoder_data->output_frames;
}
