#     
#  Makefile for Shine on Alchemy by kikko.fr 2010
#

                               
CC = gcc

SOURCES = main.c \
	wave.c  \
	tables.c    \
	layer3.c    \
	l3subband.c \
	l3bitstream.c \
	formatBitstream.c \
	l3mdct.c    \
	l3loop.c    \
	reservoir.c \
	error.c \
	
OTHERC =ieeefloat.c  \
	portableio.c \
	bitstream.c  \
	huffman.c   

OBJ = $(SOURCES:.c=.o) $(OTHERC:.c=.o)

CFLAGS += -O3 -Wall
CC_SWITCHES = -O9 -Wall
PNAME = shine

swc: $(OBJ)
	$(CC) $(CFFLAGS) -swc -o $(PNAME).swc $(OBJ);

todo:
	@echo "TODO :"
	@echo "handle bitdepths (only 16 bits now)"
	@echo "handle sample rates (only 44000Hz now)"
	@echo "handle channels (only stereo now)"

%.o: %.c 
	$(CC) $(CC_SWITCHES) -c $< -o $@

clean:
	rm -f $(OBJ) core $(PNAME) *.bak


# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: types.h portableio.h ieeefloat.h error.h wave.h layer3.h
wave.o: wave.h error.h
tables.o: tables.h types.h portableio.h ieeefloat.h
layer3.o: layer3.h ieeefloat.h types.h wave.h error.h l3subband.h l3mdct.h l3loop.h
l3subband.o: l3subband.h portableio.h ieeefloat.h tables.h 
l3bitstream.o: l3bitstream.h types.h portableio.h ieeefloat.h bitstream.h layer3.h l3mdct.h l3loop.h formatBitstream.h huffman.h tables.h error.h
formatBitstream.o: formatBitstream.h
l3mdct.o: l3mdct.h
l3loop.o: l3loop.h types.h portableio.h ieeefloat.h error.h tables.h layer3.h huffman.h l3bitstream.h bitstream.h reservoir.h
reservoir.o: reservoir.h types.h portableio.h ieeefloat.h error.h layer3.h l3loop.h huffman.h l3bitstream.h bitstream.h
error.o : error.h