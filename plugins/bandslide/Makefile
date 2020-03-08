include ../../global_config

OBJS = 	bandslide.o  \
		 bandslidewin.o

OUTPUT = ../bandslide.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
