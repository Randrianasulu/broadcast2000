#########################################################
#                         PARAMETRIC EQ
#########################################################

include ../../global_config

OBJS = 		parametric.o  \
		parametricwindow.o

OUTPUT = ../eqparametric.plugin

$(OUTPUT): $(OBJS)
	$(CC) $(PLUGINLFLAGS) -o $(OUTPUT) $(OBJS) $(PLUGINAUDIOOBJS)

include ../../plugin_config
