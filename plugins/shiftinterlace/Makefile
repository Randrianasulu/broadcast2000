#########################################################
#                         INVERT VIDEO
#########################################################

include ../../global_config

OBJS = shiftinterlace.o  \
		 shiftwindow.o

OUTPUT = ../shiftinterlace.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
