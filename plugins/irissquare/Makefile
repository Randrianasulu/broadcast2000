include ../../global_config

OBJS = 	irissquare.o  \
		 irissquarewin.o

OUTPUT = ../irissquare.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
