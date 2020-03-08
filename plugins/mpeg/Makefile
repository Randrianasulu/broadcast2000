include ../../global_config

OBJS = mpeg.o \
	mpegwindow.o \
	overlayframe.o

AUDIOOBJS = \


OUTPUT = ../mpeg.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) -la52 libmpeg3/libmpeg3.a $(PLUGINIOOBJS)



include ../../plugin_config
