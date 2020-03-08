include ../../global_config

OBJS = blur.o  \
		 blurwindow.o

OUTPUT = ../blur.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
