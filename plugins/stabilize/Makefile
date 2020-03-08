include ../../global_config

OBJS = stabilize.o  \
		 stabilizewindow.o

OUTPUT = ../stabilize.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
