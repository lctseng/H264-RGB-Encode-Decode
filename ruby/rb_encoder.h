#include "stdbool.h"

#include "ruby.h"
#include "lib/encoder.h"

VALUE rb_encoder_init(VALUE self, VALUE Qwidth, VALUE Qheight);
VALUE rb_encoder_cleanup(VALUE self);

VALUE rb_encoder_encode(VALUE self, VALUE rb_raw_data);
