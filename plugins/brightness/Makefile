include ../../global_config

OBJS = brightness.o  \
		 brightwindow.o

OUTPUT = ../brightness.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
