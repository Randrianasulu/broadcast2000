include ../global_config
export CFLAGS

OBJS = \
	bcbitmap.o \
	bcbutton.o \
	bccanvas.o \
	bccapture.o \
	bcfilebox.o \
	bcipc.o \
	bclistbox.o \
	bcmeter.o \
	bcmenubar.o \
	bcpan.o \
	bcpopup.o \
	bcpopupmenu.o \
	bcpots.o \
	bcprogress.o \
	bcprogressbox.o \
	bcrepeater.o \
	bcresources.o \
	bcscrollbar.o \
	bcsliders.o \
	bcsubwindow.o \
	bctextarea.o \
	bctextbox.o \
	bctitle.o \
	bctool.o \
	bctoggles.o \
	bcwindow.o \
	bcwindowbase.o \
	errorbox.o \
	filesystem.o \
	defaults.o \
	mutex.o \
	sema.o \
	stringfile.o \
	thread.o \
	timer.o \
	units.o \
	vframe.o

OUTPUT = libbcbase.so
STATICOUTPUT = libbcbase.a

# FOR SOME REASON PTHREAD DOESN'T WORK WHEN LINKED HERE

$(OUTPUT): $(OBJS)
	$(LDLINKER) $(LFLAGS) -o $(OUTPUT) $(OBJS)
	ar rcs $(STATICOUTPUT) $(OBJS) $(ALPHAOBJS)

static: $(OBJS)
	ar rcs $(STATICOUTPUT) $(OBJS) $(ALPHAOBJS)

test:  test.o
	$(CC) -o test test.o libbcbase.a $(LFLAGS) $(XLIBS)

.C.o:
	$(CC) -c `./c_flags` $*.C

clean:
	rm -f core
	rm -f *.o
	rm -rf cxx_repository
	rm -f $(OUTPUT) $(STATICOUTPUT)
	rm -f test test2

wc:
	cat *.C *.h | wc
