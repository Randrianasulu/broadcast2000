#########################################################
#                         LEVEL
#########################################################

include ../../global_config

OBJS = invert.o  \
		 invertwindow.o

OUTPUT = ../invertaudio.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
