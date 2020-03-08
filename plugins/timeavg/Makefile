include ../../global_config

OBJS = timeavg.o  \
		 timeavgwindow.o

OUTPUT = ../timeavg.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
