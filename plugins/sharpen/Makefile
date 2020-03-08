include ../../global_config

OBJS = sharpen.o  \
		 sharpenwindow.o

OUTPUT = ../sharpen.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS)

include ../../plugin_config
