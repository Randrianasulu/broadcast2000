include ../../global_config

OBJS = 	slide.o  \
		 slidewin.o

OUTPUT = ../slide.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
