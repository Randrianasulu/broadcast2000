#########################################################
#                         LEVEL
#########################################################

include ../../global_config

OBJS = rgb601.o  \
		 rgb601window.o

OUTPUT = ../rgb601.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
