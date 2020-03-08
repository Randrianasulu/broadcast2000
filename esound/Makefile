include ../global_config

OBJS = \
	audio.o \
	clients.o \
	esd_config.o \
	esdfile.o \
	esdlib.o \
	esdmgr.o \
	genrand.o \
	filter.o \
	mix.o \
	players.o \
	proto.o \
	samples.o



OUTPUT = libesound.a
CC = gcc
CFLAGS += -I../audiofile -DSYSCONFDIR=\"/usr/local/etc\"

$(OUTPUT): $(OBJS)
	ar rc $(OUTPUT) $(OBJS)

clean:
	rm -f *.o core

.c.o:
	$(CC) -c $(CFLAGS) $*.c
