CC = gcc
CFLAGS = -g -Wall -Werror -std=c99 -m64
TRANS_CFLAGS = $(CFLAGS) -O0

.PHONY: all clean clean-traces test32 test64

all: csim-ref trans.o test-trans tracegen

trans.o: trans.c cachelab.h
	$(CC) $(CFLAGS) -o trans.o -c trans.c

test-trans: test-trans.c cachelab.h cachelab.c trans.o
	$(CC) $(CFLAGS) -o test-trans test-trans.c cachelab.c trans.o

tracegen: tracegen.c cachelab.h cachelab.c trans.o
	$(CC) $(CFLAGS) -o tracegen tracegen.c cachelab.c trans.o

csim-ref: csim-ref.c cachelab.h cachelab.c
	$(CC) $(CFLAGS) -o csim-ref csim-ref.c cachelab.c -lm

test32: test-trans csim-ref tracegen
	./test-trans -M 32 -N 32

test64: test-trans csim-ref tracegen
	./test-trans -M 64 -N 64

testfinal: test-trans csim-ref tracegen
	./test-trans -M 61 -N 67

clean-traces:
	rm -f trans trace.tmp trace.f* .csim_results .marker

clean: clean-traces
	rm -f csim csim-ref test-csim test-trans tracegen trans.o info result