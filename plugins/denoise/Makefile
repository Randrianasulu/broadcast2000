#########################################################
#                         COMPRESSOR
#########################################################

include ../../global_config

OBJS = denoise.o  \
	denoisewindow.o

OUTPUT = ../denoise.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
