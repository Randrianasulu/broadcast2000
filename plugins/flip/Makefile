#########################################################
#                         LEVEL
#########################################################

include ../../global_config

OBJS = flip.o  \
		 flipwindow.o

OUTPUT = ../flip.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
