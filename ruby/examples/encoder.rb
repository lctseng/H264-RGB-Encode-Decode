require '../h264_rgb'

if ARGV.size < 4
  puts "Usage: #{__FILE__} [width] [height] [in_file.rgb] [out_file.h264]"
  exit
end

width = ARGV[0].to_i
height = ARGV[1].to_i
in_file_name = ARGV[2]
out_file_name = ARGV[3]

in_buf = "0" * (width * height * 3) 

out_file = File.open(out_file_name, "wb")

count = 0

encoder = H264_RGB::Encoder.new(width, height)
File.open(in_file_name, "rb") do |f|
  while len = f.read(in_buf.bytesize, in_buf)
    output = encoder.encode(in_buf)
    unless output.empty?
      count += 1
      puts "Written [#{count}]: #{output.bytesize}"
      out_file.write output
    end
  end
end

out_file.close
