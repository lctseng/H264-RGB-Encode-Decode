// Include the Ruby headers and goodies
#include "ruby.h"

// Defining a space for information and references about the module to be stored internally
VALUE H264_RGB = Qnil;

// Our 'test1' method.. it simply returns a value of '10' for now.
VALUE method_test1(VALUE self) {
	int x = 10;
	return INT2NUM(x);
}

// The initialization method for this module
void Init_h264_rgb() {
	H264_RGB = rb_define_module("H264_RGB");
	rb_define_method(H264_RGB, "test1", method_test1, 0);
}
