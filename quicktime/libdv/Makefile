include ../global_config

LFLAGS += $(shell glib-config --libs) $(shell gtk-config --libs)
LIBS = -lm -lraw1394 -lpthread

OBJS = \
	bitstream.o \
	dct.o \
	idct_248.o \
	libdv.o \
	place.o \
	parse.o \
	quant.o \
	vlc.o \
	weighting.o \
	ycrcb_to_rgb32.o

all: $(OBJS) $(DVMMX_OBJS2)

playdv: playdv.o $(OBJS)
	gcc -o playdv playdv.o $(OBJS) $(DVMMX_OBJS2) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) -c `./c_flags` $*.c

.S.o:
	$(CC) -c `./c_flags` $*.S

clean:
	rm -f *.o *.a playdv core
