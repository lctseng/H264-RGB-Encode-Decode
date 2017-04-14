#include "stdbool.h"

#include "ruby.h"
#include "lib/decoder.h"


VALUE rb_decoder_initialize(VALUE self);
VALUE rb_decoder_alloc(VALUE self);
VALUE rb_decoder_dispose(VALUE self);

VALUE rb_decoder_parse(VALUE self, VALUE in_buffer);
VALUE rb_decoder_flush(VALUE self);
