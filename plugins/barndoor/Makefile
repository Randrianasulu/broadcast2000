include ../../global_config

OBJS = 	barndoor.o  \
		 barndoorwin.o

OUTPUT = ../barndoor.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
