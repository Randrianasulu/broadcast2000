#########################################################
#                         SAMPLERATE CONVERTER
#########################################################

include ../../global_config

OBJS = samplerate.o \
			 sratewindow.o

OUTPUT = ../samplerate.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
