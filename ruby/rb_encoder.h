#include "stdbool.h"

#include "ruby.h"
#include "lib/encoder.h"

VALUE rb_encoder_initialize(VALUE self, VALUE Qwidth, VALUE Qheight);
VALUE rb_encoder_dispose(VALUE self);

VALUE rb_encoder_encode(VALUE self, VALUE rb_raw_data);

VALUE rb_encoder_alloc(VALUE self);
