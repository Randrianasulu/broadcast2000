#########################################################
#                         VREVERSE
#########################################################

include ../../global_config

OBJS = vreverse.o

OUTPUT = ../reversevideo.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
