include ../../global_config

OBJS = 	wipe.o  \
		 wipewin.o

OUTPUT = ../wipe.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
