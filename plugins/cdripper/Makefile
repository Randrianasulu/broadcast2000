#########################################################
#                         SAMPLERATE CONVERTER
#########################################################

include ../../global_config

OBJS = cdripper.o \
			 cdripwindow.o

OUTPUT = ../cdripper.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
