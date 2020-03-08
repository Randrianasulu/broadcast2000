#########################################################
#                         COMPRESSOR
#########################################################

include ../../global_config

OBJS = compress.o  \
	compresswindow.o \
	confirmsave.o

OUTPUT = ../compress.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
