include ../global_config

DIRS = \
	blur \
	colors \
    	colorbalance \
	dbx \
	fourier \
	freezeframe \
	areverse \
	bandslide \
	bandwipe \
	barndoor \
	bluescreen \
	blur \
	brightness \
	cdripper \
	compress \
	convolve \
	delay \
	denoise \
	deinterlace \
	destabilize \
	flip \
	framerate \
	freeverb \
	graphic \
	huesaturation \
	invert \
	invertvideo \
	irissquare \
	level \
	mpeg \
	normalize \
	offsetdc \
	oilpainting \
	parametric \
	pitch \
	polar \
	reverb \
	rgb601 \
    	rotate \
	samplerate \
	scale \
	sharpen \
	shiftinterlace \
	slide \
	spectrogram \
	stabilize \
	swapchannels \
	synthesizer \
	timeavg \
	timestretch \
	vreverse \
	whirl \
	wipe \
	yuv

export CFLAGS

all: $(DIRS) $(OBJS)
	@ for i in $(DIRS) ; \
	do \
		 $(MAKE) -C $$i ; \
	done

.C.o:
	$(CC) -c `c_flags` $*.C

clean:
	rm -f core
	rm -f *.o
		@ for i in $(DIRS) ; \
	do \
		 $(MAKE) clean -C $$i ; \
	done

install:
	echo "no install for plugins"

wc:
	cat *.C *.h | wc
	$(LOOP_BEGIN) \
		 $(MAKE) -C $$i wc; \
	$(LOOP_END)
