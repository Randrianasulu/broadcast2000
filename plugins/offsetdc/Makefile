#########################################################
#                         OFFSET DC
#########################################################

include ../../global_config

OBJS = offsetdc.o

OUTPUT = ../offsetdc.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
