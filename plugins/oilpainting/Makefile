include ../../global_config

OBJS = oil.o  \
		 oilwindow.o

OUTPUT = ../oil.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
