#include "rb_decoder.h"

static struct {
  bool flag_init;
  VALUE output_frames;

  VALUE padding_str;
} decoder_data = {.flag_init = false, .output_frames = Qnil, .padding_str = Qnil };

static void on_frame_ready(uint8_t* frame_buf, int frame_size){
  // make the data as string and push into array
  VALUE frame = rb_str_new(frame_buf, frame_size);
  rb_ary_push(decoder_data.output_frames, frame);
}

VALUE rb_decoder_init(VALUE self){
  if(!decoder_data.flag_init){
    if(decoder_init() < 0){
      // fail
      return Qfalse;
    }
    else{
      decoder_data.flag_init = true;
      decoder_set_frame_ready_handler(on_frame_ready);
      decoder_data.output_frames = rb_ary_new();
      // create padding str
      int size = decoder_input_buffer_padding_size();
      char* buf = malloc(sizeof(char) * size);
      memset(buf, 0, size);
      decoder_data.padding_str = rb_str_new(buf, size);
      free(buf);
      // return true
      return Qtrue;
    }
  }
  else{
    return Qtrue;
  }
}

VALUE rb_decoder_cleanup(VALUE self){
  if(decoder_data.flag_init){
    decoder_cleanup();
    decoder_data.flag_init = false;
    decoder_data.output_frames = Qnil;
    decoder_data.padding_str = Qnil;
    return Qtrue;
  }
  else{
    return Qfalse;
  }
}

VALUE rb_decoder_parse(VALUE self, VALUE in_buffer){
  if(!decoder_data.flag_init){
    rb_raise(rb_eRuntimeError, "Decoder not initialized");
  }
  int data_size = RSTRING_LEN(in_buffer);
  // ensure the buffer size > padding size
  // if not, enlarge it
  if(data_size < decoder_input_buffer_padding_size()){
    rb_str_buf_append(in_buffer, decoder_data.padding_str);
  }
  // clean up output frames
  rb_ary_clear(decoder_data.output_frames);
  // inject data
  decoder_parse(StringValuePtr(in_buffer),data_size);
  // return frames array
  return decoder_data.output_frames;
}
VALUE rb_decoder_flush(VALUE self){
  if(!decoder_data.flag_init){
    rb_raise(rb_eRuntimeError, "Decoder not initialized");
  }
  // clean up output frames
  rb_ary_clear(decoder_data.output_frames);
  decoder_parse(NULL, 0);
  decoder_flush();
  // return frames array
  return decoder_data.output_frames;
}
