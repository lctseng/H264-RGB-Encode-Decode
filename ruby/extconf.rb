# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Give it a name
extension_name = 'h264_rgb'

$srcs = ["h264_rgb.c", "rb_encoder.c", "rb_decoder.c", "lib/decoder.c", "lib/encoder.c"]
$VPATH << "./lib"

$CFLAGS += " -Wno-deprecated-declarations"

$LIBS += " -lx264 -lswscale -lavcodec -lavutil"

# The destination
dir_config(extension_name)

# Do the work
create_makefile(extension_name)
