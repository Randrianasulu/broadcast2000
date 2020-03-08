#### BROADCAST 2000
#
# LINKING IN -lXThrStub CAUSED MUTEXES TO FAIL

include ../global_config


#OBJECTS USED BY BROADCAST 2000
OBJS = \
	adeviceprefs.o \
	adrivermenu.o \
	aedit.o \
	aedits.o \
	amodule.o \
	aplugin.o \
	apluginarray.o \
	arender.o \
	arenderthread.o \
	assetedit.o \
	assetmanager.o \
	assets.o \
	atrack.o \
	attachmentpoint.o \
	audio1394.o \
	audioalsa.o \
	audioconfig.o \
	audiodevice.o \
	audioidevice.o \
	audioodevice.o \
	audioesound.o \
	audiooss.o \
	autoconf.o \
	auto.o \
	autos.o \
	bezierauto.o \
	bezierautos.o \
	bitspopup.o \
	browsebutton.o \
	buttonbar.o \
	cache.o \
	channel.o \
	channeledit.o \
	channelpicker.o \
	chantables.o \
	commonrender.o \
	commonrenderthread.o \
	compresspopup.o \
	confirmsave.o \
	confirmquit.o \
	console.o \
	consolescroll.o \
	cropvideo.o \
	cursor.o \
	dcoffset.o \
	deleteallindexes.o \
	edit.o \
	edits.o \
	featheredits.o \
	file.o \
	filebase.o \
	filebaseima4.o \
	filebaseulaw.o \
	filebasevideo.o \
	fileformat.o \
	filejpeg.o \
	filejpeglist.o \
	filemov.o \
	filepcm.o \
	fileplugin.o \
	filepng.o \
	filethread.o \
	filetiff.o \
	floatauto.o \
	floatautos.o \
	formatcheck.o \
	formatpopup.o \
	formattools.o \
	formatwindow.o \
	indexfile.o \
	indexprefs.o \
	indexthread.o \
	labels.o \
	levelwindow.o \
	levelwindowgui.o \
	loadfile.o \
	main.o \
	mainmenu.o \
	mainsamplescroll.o \
	mainundo.o \
	mainwindow.o \
	mainwindowgui.o \
	menuaeffects.o \
	menueffects.o \
	menuveffects.o \
	mwindowedit.o \
	mwindowmove.o \
	modules.o \
	module.o \
	new.o \
	neworappend.o \
	okbutton.o \
	overlayframe.o \
	patch.o \
	patchbay.o \
	playabletracks.o \
	playbackcursor.o \
	playbackengine.o \
	playbackprefs.o \
	plugin.o \
	pluginarray.o \
	plugindialog.o \
	pluginpopup.o \
	pluginprefs.o \
	pluginserver.o \
	pluginioserver.o \
	preferences.o \
	preferencesthread.o \
	previewaudio.o \
	previewvideo.o \
	progressbox.o \
	question.o \
	quit.o \
	recconfirmdelete.o \
	record.o \
	recordableatracks.o \
	recordablevtracks.o \
	recordaudio.o \
	recordengine.o \
	recordgui.o \
	recordlabel.o \
	recordprefs.o \
	recordpreview.o \
	recordthread.o \
	recordtransport.o \
	recordvideo.o \
	recvideowindow.o \
	render.o \
	renderengine.o \
	savefile.o \
	scale.o \
	setchannels.o \
	setframerate.o \
	setsamplerate.o \
	sharedpluginlocation.o \
	statusbar.o \
	threadindexer.o \
	threadloader.o \
	timebar.o \
	timebomb.o \
	toggleauto.o \
	toggleautos.o \
	trackcanvas.o \
	track.o \
	tracks.o \
	trackscroll.o \
	tracksdraw.o \
	tracksedit.o \
	transition.o \
	transitionpopup.o \
	undostack.o \
	vdevice1394.o \
	vdevicebase.o \
	vdevicex11.o \
	vdevicelml.o \
	vdeviceprefs.o \
	vedit.o \
	vedits.o \
	videoconfig.o \
	videodevice.o \
	videoprefs.o \
	videowindow.o \
	videowindowgui.o \
	viewprefs.o \
	virtualnode.o \
	virtualanode.o \
	virtualvnode.o \
	vmodule.o \
	vplugin.o \
	vpluginarray.o \
	vrender.o \
	vrenderthread.o \
	vtrack.o

OUTPUT = bcast2000

#LINUX
STATICLIBS = ../pluginpack/sharedpluginobjs.a \
	../quicktime/libquicktime.a \
        ../audiofile/libasound.a \
        ../esound/libesound.a \
	../tiff/libtiff.a

# LINUX
LIBS = -L../bcbase/ -lbcbase $(XLIBS) -lpng -lz -lglib -ldl

CFLAGS += -I../bcbase \
	-I../pluginpack \
	-I../quicktime \
	-I../quicktime/libdv \
	-I../audiofile \
	-I../esound \
	$(shell glib-config --cflags)

export CFLAGS

$(OUTPUT): $(OBJS)
	$(CC) -o $(OUTPUT) $(OBJS) $(STATICLIBS) $(LFLAGS) $(LIBS)
#	strip $(OUTPUT)
#	$(CC) -o $(OUTPUT) $(OBJS) $(STATICLIBS) $(LFLAGS) $(LIBS) ../LeakTracer.o

test: test.o
	$(CC) -o test test.o $(STATICLIBS) $(LFLAGS) $(LIBS)

.C.o:
	$(CC) -c `./c_flags` $*.C

clean:
	rm -f core
	rm -f *.o *.a
	rm -f $(OUTPUT)
	rm -rf cxx_repository

wc:
	cat *.C *.h | wc

install:
	cp $(OUTPUT) $(PREFIX)/bin
