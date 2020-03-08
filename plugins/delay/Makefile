#########################################################
#                         DELAY
#########################################################

include ../../global_config

OBJS = delay.o  \
		 delaywindow.o

OUTPUT = ../delay.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
