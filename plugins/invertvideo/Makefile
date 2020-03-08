#########################################################
#                         INVERT VIDEO
#########################################################

include ../../global_config

OBJS = invert.o  \
		 invertwindow.o

OUTPUT = ../invertvideo.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
