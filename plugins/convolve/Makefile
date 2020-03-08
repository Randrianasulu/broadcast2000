include ../../global_config

OBJS = convolve.o  \
	convolvewindow.o

OUTPUT = ../convolve.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
