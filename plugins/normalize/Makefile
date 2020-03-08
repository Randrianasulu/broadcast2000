#########################################################
#                         NORMALIZE
#########################################################

include ../../global_config

OBJS = normalize.o \
	normalizewindow.o

OUTPUT = ../normalize.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
