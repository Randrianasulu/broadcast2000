#########################################################
#                         COMPRESSOR
#########################################################

include ../../global_config

OBJS = confirmsave.o \
	synthesizer.o  \
	synthmenu.o \
	synthwindow.o

OUTPUT = ../synthesizer.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
