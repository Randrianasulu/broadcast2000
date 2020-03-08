include ../../global_config

OBJS = yuv.o  \
		 yuvwindow.o

OUTPUT = ../yuv.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINVIDEOOBJS) ../colors/colors.o

include ../../plugin_config
