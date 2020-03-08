#### THE MASTER MAKEFILE

DIRS = libmpeg3 \
	tiff \
	esound \
	audiofile \
	quicktime \
	bcbase \
	guicast \
	pluginpack \
	bcast \
	plugins
DISTNAME = bcast2000demo.tar
SOURCENAME = heroinesrc.tar
DISTOBJS = \
	$(INSTDIR)/bcast/bcast2000 \
	$(INSTDIR)/plugins/*.plugin


include global_config

all: ipc soundtest
	@ for i in $(DIRS) ; \
	do \
		 $(MAKE) -j 2 -C $$i ; \
	done

ipc: ipc.o
	$(CC) -o ipcclear ipc.o

soundtest: soundtest.o
	$(CC) -o soundtest soundtest.o -lm

backup: clean
	cd .. && \
	tar Icvf cinelerra.tar.bz2 cinelerra

.C.o:
	$(CC) -c $(CFLAGS) -Ibcbase $*.C

clean:
	rm -f *.o ipcclear soundtest
	make -C cinelerra clean
	make -C plugins clean
	@ for i in $(DIRS) ; \
	do \
		 $(MAKE) -C $$i clean; \
	done

wc:
	cat *.C *.h | wc
	@ for i in $(DIRS) ; \
	do \
		 $(MAKE) -C $$i wc; \
	done

# From make_packages
install:
	mkdir -p /usr/local/bcast/plugins
	mkdir -p /usr/local/bin
	cp bcast/bcast2000 /usr/local/bcast
	cp bcbase/libbcbase.so /usr/local/bcast
	cp guicast/libguicast.so /usr/local/bcast
	strip plugins/*.plugin
	cp plugins/*.plugin /usr/local/bcast/plugins
	cp bcast2000.sh /usr/local/bcast
	cp record_script /usr/local/bcast
	cp -rd docs /usr/local/bcast
	cd /usr/local/bin
	ln -sf ../bcast/bcast2000.sh bcast

