include ../../global_config

OBJS = rotate.o  \
		 rotatewindow.o

OUTPUT = ../rotate.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
