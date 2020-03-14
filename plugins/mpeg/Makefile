include ../../global_config

OBJS = mpeg.o \
	mpegwindow.o \
	overlayframe.o

AUDIOOBJS = \

OBJDIR := $(shell uname --machine)

OUTPUT = ../mpeg.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) -la52 libmpeg3/$(OBJDIR)/libmpeg3.a $(PLUGINIOOBJS)



include ../../plugin_config
