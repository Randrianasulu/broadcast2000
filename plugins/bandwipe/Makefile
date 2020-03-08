include ../../global_config

OBJS = 	bandwipe.o  \
		 bandwipewin.o

OUTPUT = ../bandwipe.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
