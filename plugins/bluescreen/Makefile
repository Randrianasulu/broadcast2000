include ../../global_config

OBJS = bluescreen.o  \
		 bluewindow.o

OUTPUT = ../chromakey.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS) ../colors/colors.o

include ../../plugin_config
