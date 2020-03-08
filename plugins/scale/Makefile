include ../../global_config

OBJS = 	overlayframe.o \
		scale.o  \
		 scalewin.o

OUTPUT = ../scale.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
