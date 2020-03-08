#########################################################
#                         REVERB s
#########################################################

include ../../global_config

OBJS = confirmsave.o \
	reverb.o  \
	reverbwindow.o

OUTPUT = ../reverb.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
