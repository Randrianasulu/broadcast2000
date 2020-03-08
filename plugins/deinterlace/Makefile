#########################################################
#                         INVERT VIDEO
#########################################################

include ../../global_config

OBJS = deinterlace.o  \
		 deinterwindow.o

OUTPUT = ../deinterlace.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
