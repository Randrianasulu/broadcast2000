include ../../global_config

OBJS = confirmsave.o \
	graphic.o  \
	graphicwindow.o

OUTPUT = ../eqgraphic.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS) ../fourier/fourier.o

include ../../plugin_config
