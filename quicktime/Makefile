include global_config

OBJS = \
	atom.o \
	codecs.o \
	colormodels.o \
	ctab.o \
	dinf.o \
	dref.o \
	dv.o \
	edts.o \
	elst.o \
	graphics.o \
	hdlr.o \
	ima4.o \
	jpeg.o \
	jpeg_old.o \
	libmjpeg.o \
	matrix.o \
	mdat.o \
	mdhd.o \
	mdia.o \
	minf.o \
	moov.o \
	mvhd.o \
	plugin.o \
	qtpng.o \
	quicktime.o \
	raw.o \
	rawaudio.o \
	smhd.o \
	sowt.o \
	stbl.o \
	stco.o \
	stsc.o \
	stsd.o \
	stsdtable.o \
	stss.o \
	stsz.o \
	stts.o \
	tkhd.o \
	trak.o \
	twos.o \
	udta.o \
	ulaw.o \
	util.o \
	vmhd.o \
	wmx2.o \
	yuv2.o \
	yuv4.o \
	yv12.o


#JPEGOBJS = \
#	jpeg/jcapimin.o \
#	jpeg/jcapistd.o \
#	jpeg/jctrans.o \
#	jpeg/jcparam.o \
#	jpeg/jdatadst.o \
#	jpeg/jcinit.o \
#	jpeg/jcmaster.o \
#	jpeg/jcmarker.o \
#	jpeg/jcmainct.o \
#	jpeg/jcprepct.o \
#	jpeg/jccoefct.o \
#	jpeg/jccolor.o \
#	jpeg/jcsample.o \
#	jpeg/jchuff.o \
#	jpeg/jcphuff.o \
#	jpeg/jcdctmgr.o \
#	jpeg/jfdctfst.o \
#	jpeg/jfdctflt.o \
#	jpeg/jfdctint.o \
#	jpeg/jdapimin.o \
#	jpeg/jdapistd.o \
#	jpeg/jdtrans.o \
#	jpeg/jdatasrc.o \
#	jpeg/jdmaster.o \
#	jpeg/jdinput.o \
#	jpeg/jdmarker.o \
#	jpeg/jdhuff.o \
#	jpeg/jdphuff.o \
#	jpeg/jdmainct.o \
#	jpeg/jdcoefct.o \
#	jpeg/jdpostct.o \
#	jpeg/jddctmgr.o \
#	jpeg/jidctfst.o \
#	jpeg/jidctflt.o \
#	jpeg/jidctint.o \
#	jpeg/jidctred.o \
#	jpeg/jdsample.o \
#	jpeg/jdcolor.o \
#	jpeg/jquant1.o \
#	jpeg/jquant2.o \
#	jpeg/jdmerge.o \
#	jpeg/jcomapi.o \
#	jpeg/jutils.o \
#	jpeg/jerror.o \
#	jpeg/jmemmgr.o \
#	jpeg/jmemnobs.o

DVOBJS = \
	libdv/bitstream.o \
	libdv/dct.o \
	libdv/idct_248.o \
	libdv/libdv.o \
	libdv/parse.o \
	libdv/place.o \
	libdv/quant.o \
	libdv/vlc.o \
	libdv/weighting.o \
	libdv/ycrcb_to_rgb32.o


OUTPUT = libquicktime.a

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
#	$(MAKE) -C jpeg
	$(MAKE) -C libdv
# objcopy -L dv_peek_vlc libdv/parse.o
	$(MAKE) -C libraw1394
	$(MAKE) -C libjpeg-turbo-1.5.3
	ar rcs $(OUTPUT) $(OBJS) $(DVOBJS) $(FIREWIREOBJS) $(DVMMXOBJS)
#	g++ -shared -o libquicktime.so  $(OBJS)  $(DVOBJS) $(FIREWIREOBJS) $(DVMMXOBJS)

qtdump: dump.o $(OUTPUT)
	$(CC) -o qtdump dump.o $(OUTPUT) -lm -lpng -lz -ljpeg -lpthread -lglib -ldl

make_streamable: make_streamable.o $(OUTPUT)
	$(CC) -o make_streamable make_streamable.o $(OUTPUT) -lm -lpng -ljpeg -lz -lpthread -lglib -ldl

dechunk: dechunk.o $(OUTPUT)
	$(CC) -o dechunk dechunk.o $(OUTPUT) -lm -lpng -ljpeg -lz -lpthread -lglib -ldl

recover: recover.o $(OUTPUT)
	$(CC) -o recover recover.o $(OUTPUT) -lm -lpng -ljpeg -lz -lpthread -lglib -ldl

yuv4toyuv: yuv4toyuv.o $(OUTPUT)
	$(CC) -o yuv4toyuv yuv4toyuv.o $(OUTPUT) -lm -lpng -ljpeg -lz -lpthread -lglib -ldl

qtinfo: qtinfo.o
	$(CC) -o qtinfo qtinfo.o $(OUTPUT) -lm -lpng -lz -lpthread -ljpeg -lglib -ldl

util:  qtdump make_streamable qtinfo dechunk yuv4toyuv recover
	

quicktime.o: quicktime.h  quicktime.c

.c.o:
	$(CC) -c `./c_flags` $*.c

clean:
	rm -f core
	rm -f *.o *.so
	rm -f $(OUTPUT)
	rm -f qtdump make_streamable a.out qtinfo dechunk yuv4toyuv recover
	rm -rf .deps
	$(MAKE) -C libdv clean
	$(MAKE) -C jpeg clean
	$(MAKE) -C libraw1394 clean

wc:
	cat *.c *.h | wc

backup: clean
	cd .. && \
	tar zcvf quicktime.tar.gz quicktime
