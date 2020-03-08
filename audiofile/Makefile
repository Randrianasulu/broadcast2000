include ../global_config

OBJS = \
	aes.o \
	aiff.o \
	aiffwrite.o \
	au.o \
	audiofile.o \
	aupv.o \
	auwrite.o \
	bread.o \
	bwrite.o \
	byteorder.o \
	compression.o \
	error.o \
	extended.o \
	g711.o \
	instrument.o \
	loop.o \
	marker.o \
	misc.o \
	query.o \
	track.o \
	ulaw.o \
	util.o \
	wave.o \
	wavewrite.o \
	write.o

OUTPUT = libasound.a
CC = gcc

$(OUTPUT): $(OBJS)
	ar rc $(OUTPUT) $(OBJS)

clean:
	rm -f *.o core *.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c
