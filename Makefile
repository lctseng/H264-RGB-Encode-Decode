all: encoder.out decoder.out
encoder.out: encoder.c
	gcc encoder.c -o encoder.out -O2 -lx264
decoder.out: decoder-example.c
	gcc decoder-example.c lib/decoder.c -o decoder.out -O2 -Wno-deprecated-declarations -lswscale -lavcodec -lavutil
clean:
	rm decoder.out
	rm encoder.out

