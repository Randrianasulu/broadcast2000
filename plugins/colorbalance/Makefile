include ../../global_config

OBJS = colorbalance.o  \
		 colorbalancewindow.o

OUTPUT = ../colorbalance.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS) ../colors/colors.o

include ../../plugin_config
