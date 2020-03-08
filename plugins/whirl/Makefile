include ../../global_config

OBJS = whirl.o  \
		 whirlwindow.o

OUTPUT = ../whirl.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
