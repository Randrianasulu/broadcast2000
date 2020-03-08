#########################################################
#                         LEVEL
#########################################################

include ../../global_config

OBJS = level.o  \
		 levelwindow.o

OUTPUT = ../level.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
