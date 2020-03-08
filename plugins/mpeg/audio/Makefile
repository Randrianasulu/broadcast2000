CC = gcc

OBJS = \
	common.o \
	encode.o \
	formatBitstream.o \
	huffman.o \
	ieeefloat.o \
	l3bitstream.o \
	l3psy.o \
	loop.o \
	mdct.o \
	musicin.o \
	portableio.o \
	psy.o \
	reservoir.o \
	subs.o \
	tonal.o

CFLAGS +=  -DUNIX -DBS_FORMAT=BINARY $(NINT_SWITCH) -DNDEBUG

LIBS = -lm

all: $(OBJS)

.C.o:
	$(CC) -c $(C_FLAGS) $*.C

clean:
	-rm $(OBJS)
