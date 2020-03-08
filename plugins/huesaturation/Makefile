include ../../global_config

OBJS = huesaturation.o  \
		 huewindow.o

OUTPUT = ../huesaturation.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS) ../colors/colors.o

include ../../plugin_config
