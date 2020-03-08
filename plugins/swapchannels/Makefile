include ../../global_config

OBJS = swapchannels.o  \
		 swapwindow.o

OUTPUT = ../swapchannels.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
