include ../../global_config

OBJS = pitch.o  \
	pitchwindow.o

OUTPUT = ../pitch.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS) ../fourier/fourier.o

include ../../plugin_config
