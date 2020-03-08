#########################################################
#                         INVERT VIDEO
#########################################################

include ../../global_config

OBJS = freezeframe.o

OUTPUT = ../freezeframe.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
