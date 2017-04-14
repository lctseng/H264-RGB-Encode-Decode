require '../h264_rgb'

if ARGV.size < 3
  puts "Usage: #{__FILE__} [width] [height] [in_file.rgb]"
  exit
end

width = ARGV[0].to_i
height = ARGV[1].to_i
in_file_name = ARGV[2]

in_buf = "0" * (width * height * 3) 
in_buf.force_encoding("ASCII-8BIT")

count = 0

rgb_stream = ""
rev_rgb_stream = ""
h264_stream = ""

rgb_stream.force_encoding("ASCII-8BIT")
h264_stream.force_encoding("ASCII-8BIT")
rev_rgb_stream.force_encoding("ASCII-8BIT")


encoder = H264_RGB::Encoder.new(width, height, true)
decoder = H264_RGB::Decoder.new

File.open(in_file_name, "rb") do |f|
  while len = f.read(in_buf.bytesize, in_buf)
    rgb_stream << in_buf
    output = encoder.encode(in_buf)
    unless output.empty?
      count += 1
      puts "Written [#{count}]: #{output.bytesize}"
      h264_stream << output
      frames = decoder.parse(output)
      frames.each do |frame|
        puts "Got frame!"
        rev_rgb_stream << frame
      end
      if count % 50 == 0
        frames = decoder.flush
        frames.each do |frame|
          puts "[Flush] Got frame!"
          rev_rgb_stream << frame
        end
        puts "=== Check Point at #{count} ===> #{rgb_stream == rev_rgb_stream}"
      end
    end
  end
end

decoder.dispose
encoder.dispose
