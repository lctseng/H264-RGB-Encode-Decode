all: encoder.out decoder.out
encoder.out: encoder-example.c lib/encoder.c lib/encoder.h
	gcc encoder-example.c lib/encoder.c -o encoder.out -O2 -lx264
decoder.out: decoder-example.c lib/decoder.c lib/decoder.h
	gcc decoder-example.c lib/decoder.c -o decoder.out -O2 -Wno-deprecated-declarations -lswscale -lavcodec -lavutil
clean:
	rm decoder.out
	rm encoder.out

