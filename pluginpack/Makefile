include ../global_config

CFLAGS += -I../bcbase

export CFLAGS

#OBJECTS SHARED BY PLUGINS AND BROADCAST 2000
SHAREDOBJS = \
	filehtal.o \
	pluginbuffer.o \
	shmemory.o \
	messages.o

CLIENTOBJS = \
	pluginclient.o \
	pluginaclient.o \
	pluginvclient.o \
	pluginioclient.o

SHAREDOUTPUT = sharedpluginobjs.a

all: $(SHAREDOBJS) $(CLIENTOBJS)
	ar src $(SHAREDOUTPUT) $(SHAREDOBJS) $(ALPHAOBJS)

.C.o:
	$(CC) -c `./c_flags` $*.C

clean:
	rm -f core
	rm -rf *.o *.a
	rm -rf cxx_repository
