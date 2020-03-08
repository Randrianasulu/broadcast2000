include ../../global_config

OBJS = polar.o  \
		 polarwindow.o

OUTPUT = ../polar.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
