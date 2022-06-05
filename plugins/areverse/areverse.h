#ifndef AREVERSE_H
#define AREVERSE_H

#include "bcbase.h"
#include "pluginaclient.h"

class AReverseMain : public PluginAClient
{
public:
	AReverseMain(int argc, char *argv[]);
	~AReverseMain();

// required for all non realtime/single channel plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int start_plugin();

	int get_parameters(); 

	const char* plugin_title();
};


#endif
