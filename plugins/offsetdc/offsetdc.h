#ifndef OFFSETDC_H
#define OFFSETDC_H

#include "bcbase.h"
#include "pluginaclient.h"

class OffsetMain : public PluginAClient
{
public:
	OffsetMain(int argc, char *argv[]);
	~OffsetMain();

// required for all non realtime/single channel plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int start_plugin();

	int get_parameters(); 

	const char* plugin_title();
};


#endif
