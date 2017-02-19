#include "stdbool.h"

#include "ruby.h"
#include "lib/decoder.h"


VALUE rb_decoder_init(VALUE self);
VALUE rb_decoder_cleanup(VALUE self);

VALUE rb_decoder_parse(VALUE self, VALUE in_buffer);
VALUE rb_decoder_flush(VALUE self);
