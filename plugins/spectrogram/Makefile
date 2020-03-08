include ../../global_config

OBJS = spectrogram.o  \
	spectrogramwin.o

OUTPUT = ../spectrogram.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS) ../fourier/fourier.o

include ../../plugin_config
