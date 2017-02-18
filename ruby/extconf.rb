# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Give it a name
extension_name = 'h264_rgb'

$srcs = ["h264_rgb.c", "decoder.c", "encoder.c"]
$VPATH << "../lib"

$CXXFLAGS += "-Wno-deprecated-declarations"

# The destination
dir_config(extension_name)

# Do the work
create_makefile(extension_name)
