CC = gcc
CFLAGS += -fPIC
NASM = nasm
USE_MMX = 0
PREFIX = 
BINDIR = 
DEST_LIB_DIR = 
INCLUDEDIR = 
A52_SHARED = 
PREFIX = 
BINDIR = 
DEST_LIB_DIR = 
INCLUDEDIR = 
A52_SHARED = 
PREFIX = 
BINDIR = 
DEST_LIB_DIR = 
INCLUDEDIR = 
A52_SHARED = 
PREFIX = /usr
BINDIR = 
DEST_LIB_DIR = 
INCLUDEDIR = 
A52_SHARED = yes
USE_CSS = 1
A52DIR := $(shell expr a52dec* )

ifeq ("$(A52_SHARED)", "")
A52_SHARED=no
endif

ifeq ("$(PREFIX)", "")
PREFIX = /usr/local
endif

ifeq ("$(BINDIR)", "")
BINDIR = $(PREFIX)/bin
endif

ifeq ("$(DEST_LIB_DIR)", "")
DEST_LIB_DIR = $(PREFIX)/lib
endif

ifeq ("$(INCLUDEDIR)", "")
INCLUDEDIR = $(PREFIX)/include
endif

ifeq ($(origin CFLAGS), environment)
HAVE_CFLAGS := y
else
HAVE_CFLAGS := n
endif

OBJDIR := $(shell uname --machine)

ifeq ($(OBJDIR), alpha)
  USE_MMX = 0
  ifneq ($(HAVE_CFLAGS), y)
    CFLAGS := -O4 -arch ev67 -ieee -accept c99_keywords -gcc_messages
  endif
endif

ifeq ($(OBJDIR), i686)
  USE_MMX = 1
  ifneq ($(HAVE_CFLAGS), y)
    CFLAGS := -fPIC -O2 -fomit-frame-pointer -falign-loops=2 -falign-jumps=2 -falign-functions=2 -I/usr/local/include
  endif
  CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
endif

ifeq ($(OBJDIR), x86_64)
  ifneq ($(HAVE_CFLAGS), y)
    CFLAGS := -fPIC -O2 -fomit-frame-pointer -I/usr/local/include
  endif
  CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
endif

ifeq ($(USE_CSS), 1)
  CFLAGS += -DHAVE_CSS
endif

ifeq ($(USE_MMX), 1)
  CFLAGS += -DHAVE_MMX
  ASMOBJS = $(OBJDIR)/video/mmxidct.o
  NASMOBJS = $(OBJDIR)/video/reconmmx.o
endif

CFLAGS += \
	-I. \
	-I$(A52DIR)/include \
	-I$(A52DIR)/liba52

CFLAGS += -g

OBJS = \
	$(OBJDIR)/audio/ac3.o \
	$(OBJDIR)/audio/dct.o \
	$(OBJDIR)/audio/huffman.o \
	$(OBJDIR)/audio/layer2.o \
	$(OBJDIR)/audio/layer3.o \
	$(OBJDIR)/audio/mpeg3audio.o \
	$(OBJDIR)/audio/pcm.o \
	$(OBJDIR)/audio/synthesizers.o \
	$(OBJDIR)/audio/tables.o \
	$(OBJDIR)/libmpeg3.o \
	$(OBJDIR)/mpeg3atrack.o \
	$(OBJDIR)/mpeg3bits.o \
	$(OBJDIR)/mpeg3css.o \
	$(OBJDIR)/mpeg3demux.o \
	$(OBJDIR)/mpeg3ifo.o \
	$(OBJDIR)/mpeg3io.o \
	$(OBJDIR)/mpeg3strack.o \
	$(OBJDIR)/mpeg3title.o \
	$(OBJDIR)/mpeg3tocutil.o \
	$(OBJDIR)/mpeg3vtrack.o \
	$(OBJDIR)/video/getpicture.o \
	$(OBJDIR)/video/headers.o \
	$(OBJDIR)/video/idct.o \
	$(OBJDIR)/video/macroblocks.o \
	$(OBJDIR)/video/mmxtest.o \
	$(OBJDIR)/video/motion.o \
	$(OBJDIR)/video/mpeg3cache.o \
	$(OBJDIR)/video/mpeg3video.o \
	$(OBJDIR)/video/output.o \
	$(OBJDIR)/video/reconstruct.o \
	$(OBJDIR)/video/seek.o \
	$(OBJDIR)/video/slice.o \
	$(OBJDIR)/video/subtitle.o \
	$(OBJDIR)/video/vlc.o \
	$(OBJDIR)/workarounds.o

DIRS := \
	$(OBJDIR)/audio \
	$(OBJDIR)/video

ifeq ("$(A52_SHARED)", "no")
include Makefile.a52

DIRS += $(A52DIRS)
endif

OUTPUT = $(OBJDIR)/libmpeg3.a
UTILS = $(OBJDIR)/mpeg3dump $(OBJDIR)/mpeg3peek $(OBJDIR)/mpeg3toc  $(OBJDIR)/mpeg3cat

SHARED = $(OBJDIR)/libmpeg3.so

PKG_CONFIG_FILE=$(OBJDIR)/libmpeg3.pc

ifeq ("$(A52_SHARED)", "no")
LIBS = -lm -lpthread 
else
LIBS = -lm -lpthread -la52
endif

$(shell if ! test -d $(OBJDIR); then mkdir -p $(OBJDIR); fi )

$(shell echo $(CFLAGS) > $(OBJDIR)/c_flags)

ifeq ("$(A52_SHARED)", "no")
$(shell echo $(A52CFLAGS) > $(OBJDIR)/a52_flags)
endif

$(shell echo $(OBJS) $(ASMOBJS) $(A52OBJS) $(NASMOBJS) > $(OBJDIR)/objs)
$(shell mkdir -p $(DIRS) )

all: $(OUTPUT) $(PKG_CONFIG_FILE) $(SHARED) $(UTILS)

$(OUTPUT): $(OBJS) $(ASMOBJS) $(NASMOBJS) $(A52OBJS)
	ar rcs $(OUTPUT) `cat $(OBJDIR)/objs`

$(SHARED): $(OBJS) $(ASMOBJS) $(NASMOBJS) $(A52OBJS)
ifeq ("$(A52_SHARED)", "no")
	$(CC) `cat $(OBJDIR)/c_flags` -shared -o $(SHARED) $(OBJS) $(ASMOBJS) $(NASMOBJS) `ls $(OBJDIR)/a52dec*/liba52/*.o` -Wl,-soname -Wl,libmpeg3.so
else
	$(CC) `cat $(OBJDIR)/c_flags` -shared -o $(SHARED) $(OBJS) $(ASMOBJS) $(NASMOBJS)  -Wl,-soname -Wl,libmpeg3.so
endif

$(OBJDIR)/mpeg3dump: $(OUTPUT) mpeg3dump.c
	$(CC) `cat $(OBJDIR)/c_flags` -o $(OBJDIR)/mpeg3dump mpeg3dump.c $(OUTPUT) $(LIBS)

$(OBJDIR)/mpeg3peek: $(OUTPUT) mpeg3peek.c
	$(CC) `cat $(OBJDIR)/c_flags` -o $(OBJDIR)/mpeg3peek mpeg3peek.c $(OUTPUT) $(LIBS)

$(OBJDIR)/mpeg3toc: $(OUTPUT) mpeg3toc.c
	$(CC) `cat $(OBJDIR)/c_flags` -o $(OBJDIR)/mpeg3toc mpeg3toc.c $(OUTPUT) $(LIBS)

$(OBJDIR)/mpeg3cat: $(OUTPUT) mpeg3cat.c
	$(CC) `cat $(OBJDIR)/c_flags` -o $(OBJDIR)/mpeg3cat mpeg3cat.c $(OUTPUT) $(LIBS)

$(OBJDIR)/mpeg2qt: $(OUTPUT)
	$(CC) `cat $(OBJDIR)/c_flags` -o $(OBJDIR)/mpeg2qt mpeg2qt.c \
		$(OUTPUT) \
		$(LIBS) \
		-I. \
		-I../quicktime \
		../quicktime/$(OBJDIR)/libquicktime.a \
		-lpng \
		-lz \
		-ldl

$(PKG_CONFIG_FILE):
	@echo -e "prefix=$(PREFIX)\n\
exec_prefix=$(PREFIX)\n\
libdir=$(PREFIX)/$(DEST_LIB_DIR)\n\
includedir=$(PREFIX)/include\n\
\n\
Name: libmpeg3\n\
Description: Decoder of various derivatives of MPEG standards\n\
Version: 1.7\n\
Requires: \n\
Conflicts: \n" > $(PKG_CONFIG_FILE)
ifeq ("$(A52_SHARED)", "no")
	@echo 'Libs: -L$${libdir} -lmpeg3 -lpthread' >> $(PKG_CONFIG_FILE)
else
	@echo 'Libs: -L$${libdir} -lmpeg3 -lpthread -la52' >> $(PKG_CONFIG_FILE)
endif
	@echo 'Cflags: -I$${includedir}' >> $(PKG_CONFIG_FILE)

install:
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(DEST_LIB_DIR) $(DESTDIR)$(INCLUDEDIR)/audio $(DESTDIR)$(INCLUDEDIR)/video $(DESTDIR)$(DEST_LIB_DIR)/pkgconfig
	install -m 755 $(UTILS) $(DESTDIR)$(BINDIR)
	install $(OUTPUT) $(DESTDIR)$(DEST_LIB_DIR)
	install $(SHARED) $(DESTDIR)$(DEST_LIB_DIR)
	install -m 644 *.h $(DESTDIR)$(INCLUDEDIR)
	install -m 644 *.inc $(DESTDIR)$(INCLUDEDIR)
	install -m 644 audio/*.h $(DESTDIR)$(INCLUDEDIR)/audio
	install -m 644 video/*.h $(DESTDIR)$(INCLUDEDIR)/video
	install -m 644 $(PKG_CONFIG_FILE) $(DESTDIR)$(DEST_LIB_DIR)/pkgconfig

clean:
	rm -rf $(OBJDIR)

backup: clean
	cd .. && \
	tar zcf libmpeg3.tar.gz libmpeg3

wc:
	cat *.c *.h audio/*.c audio/*.h video/*.c video/*.h | wc

$(OBJS): 
	$(CC) -c `cat $(OBJDIR)/c_flags` $(subst $(OBJDIR)/,, $*.c) -o $*.o
$(ASMOBJS): 
	$(CC) -c `cat $(OBJDIR)/c_flags` $(subst $(OBJDIR)/,, $*.S) -o $*.o
$(NASMOBJS): 
	$(NASM) -f elf $(subst $(OBJDIR)/,, $*.s) -o $*.o

ifeq ("$(A52_SHARED)", "no")
$(A52OBJS):
	$(CC) -c `cat $(OBJDIR)/a52_flags` $(subst $(OBJDIR)/,, $*.c) -o $*.o
endif

$(OBJDIR)/libmpeg3.o: 				    libmpeg3.c
$(OBJDIR)/mpeg3atrack.o: 			    mpeg3atrack.c
$(OBJDIR)/mpeg3bits.o:  			    mpeg3bits.c
$(OBJDIR)/mpeg3css.o: 				    mpeg3css.c
$(OBJDIR)/mpeg3demux.o: 			    mpeg3demux.c
$(OBJDIR)/mpeg3dump.o: 				    mpeg3dump.c
$(OBJDIR)/mpeg3ifo.o: 				    mpeg3ifo.c
$(OBJDIR)/mpeg3io.o: 				    mpeg3io.c
$(OBJDIR)/mpeg3strack.o:                            mpeg3strack.c
$(OBJDIR)/mpeg3title.o: 			    mpeg3title.c
$(OBJDIR)/mpeg3toc3.o:  			    mpeg3toc3.c
$(OBJDIR)/mpeg3toc.o: 				    mpeg3toc.c
$(OBJDIR)/mpeg3tocutil.o:                           mpeg3tocutil.c
$(OBJDIR)/mpeg3vtrack.o: 			    mpeg3vtrack.c
$(OBJDIR)/audio/ac3.o:  			    audio/ac3.c
$(OBJDIR)/audio/bit_allocation.o: 		    audio/bit_allocation.c
$(OBJDIR)/audio/dct.o:  			    audio/dct.c
$(OBJDIR)/audio/exponents.o: 			    audio/exponents.c
$(OBJDIR)/audio/header.o: 			    audio/header.c
$(OBJDIR)/audio/huffman.o: 			    audio/huffman.c
$(OBJDIR)/audio/layer2.o: 			    audio/layer2.c
$(OBJDIR)/audio/layer3.o: 			    audio/layer3.c
$(OBJDIR)/audio/mantissa.o: 			    audio/mantissa.c
$(OBJDIR)/audio/mpeg3audio.o: 			    audio/mpeg3audio.c
$(OBJDIR)/audio/pcm.o:  			    audio/pcm.c
$(OBJDIR)/audio/synthesizers.o: 		    audio/synthesizers.c
$(OBJDIR)/audio/tables.o: 			    audio/tables.c
$(OBJDIR)/video/getpicture.o: 			    video/getpicture.c
$(OBJDIR)/video/headers.o: 			    video/headers.c
$(OBJDIR)/video/idct.o: 			    video/idct.c
$(OBJDIR)/video/macroblocks.o:  		    video/macroblocks.c
$(OBJDIR)/video/mmxtest.o: 			    video/mmxtest.c
$(OBJDIR)/video/motion.o: 			    video/motion.c
$(OBJDIR)/video/mpeg3cache.o: 			    video/mpeg3cache.c
$(OBJDIR)/video/mpeg3video.o: 			    video/mpeg3video.c
$(OBJDIR)/video/output.o: 			    video/output.c
$(OBJDIR)/video/reconstruct.o:  		    video/reconstruct.c
$(OBJDIR)/video/seek.o: 			    video/seek.c
$(OBJDIR)/video/slice.o: 			    video/slice.c
$(OBJDIR)/video/subtitle.o:                         video/subtitle.c
$(OBJDIR)/video/vlc.o:  			    video/vlc.c
$(OBJDIR)/workarounds.o:  			    workarounds.c

ifeq ("$(A52_SHARED)", "no")
include depend.a52
endif
