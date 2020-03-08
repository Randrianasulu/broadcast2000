include ../../global_config

OBJS = dbx.o  \
		 dbxwindow.o

OUTPUT = ../dbx.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config2
