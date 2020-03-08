#########################################################
#                         FRAMERATE
#########################################################

include ../../global_config

OBJS = framerate.o \
	fratewindow.o

OUTPUT = ../framerate.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
