include ../../global_config

OBJS = timestretch.o \
		timewindow.o

OUTPUT = ../timestretch.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS) ../fourier/fourier.o

include ../../plugin_config
