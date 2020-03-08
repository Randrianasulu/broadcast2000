include ../../global_config

OBJS = destabilize.o  \
		 destabilizewin.o

OUTPUT = ../destabilize.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
