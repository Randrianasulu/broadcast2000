include ../global_config
CFLAGS +=
export CFLAGS


OBJS = \
	bcbitmap.o \
	bcbutton.o \
	bccapture.o \
	bcclipboard.o \
	bcdisplayinfo.o \
	bcdragwindow.o \
	bcfilebox.o \
	bcipc.o \
	bclistbox.o \
	bcmenubar.o \
	bcmeter.o \
	bcpan.o \
	bcpixmap.o \
	bcpopup.o \
	bcpopupmenu.o \
	bcpot.o \
	bcprogress.o \
	bcprogressbox.o \
	bcrepeater.o \
	bcresources.o \
	bcscrollbar.o \
	bcsignals.o \
	bcslider.o \
	bcsubwindow.o \
	bctextbox.o \
	bctitle.o \
	bctoggle.o \
	bctumble.o \
	bcwindow.o \
	bcwindowbase.o \
	bcwindowdraw.o \
	errorbox.o \
	defaults.o \
	filesystem.o \
	mutex.o \
	sema.o \
	stringfile.o \
	thread.o \
	timer.o \
	units.o \
	vframe.o 

OUTPUT = libguicast.so
STATICOUTPUT = libguicast.a

# FOR SOME REASON PTHREAD DOESN'T WORK WHEN LINKED HERE
$(OUTPUT): $(OBJS)
	$(LDLINKER) $(LFLAGS) -o $(OUTPUT) $(OBJS) $(ALPHAOBJS)
	ar rcs $(STATICOUTPUT) $(OBJS) $(ALPHAOBJS)

static: $(OBJS)
	ar rcs $(STATICOUTPUT) $(OBJS) $(ALPHAOBJS)

replace:  replace.o $(OUTPUT)
	$(CC) -o replace replace.o libguicast.a ../quicktime/libquicktime.a $(LFLAGS) $(XLIBS) -lpng

test:  test.o $(OUTPUT)
	$(CC) -o test test.o libguicast.a ../libmpeg3/libmpeg3.a ../quicktime/libquicktime.a $(LFLAGS) $(LIBS) -lpng

.C.o:
	$(CC) -c `./c_flags` $*.C

clean:
	rm -f core
	rm -rf *.o
	rm -rf cxx_repository
	rm -f $(OUTPUT) $(STATICOUTPUT)
	rm -f test test2

wc:
	cat *.C *.h | wc

backup: clean
	cd .. && \
	tar Icvf ~/guicast.tar.bz2 guicast
