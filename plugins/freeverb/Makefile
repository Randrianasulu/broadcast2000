#########################################################
#                         REVERB s
#########################################################

include ../../global_config

BUILDOBJS = freeverb.o  \
	freeverbwin.o \
	Components/allpass.o \
	Components/comb.o \
	Components/revmodel.o

LINKOBJS = freeverb.o  \
	freeverbwin.o \
	allpass.o \
	comb.o \
	revmodel.o

OUTPUT = ../freeverb.plugin

$(OUTPUT): $(BUILDOBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(LINKOBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config2
