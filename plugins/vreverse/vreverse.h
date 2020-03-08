#ifndef VREVERSE_H
#define VREVERSE_H

#include "bcbase.h"
#include "pluginvclient.h"

class VReverseMain : public PluginVClient
{
public:
	VReverseMain(int argc, char *argv[]);
	~VReverseMain();

// required for all non realtime/single channel plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int start_plugin();

	int get_parameters(); 
	int swap_frames(VFrame *frame1, VFrame *frame2);

	char* plugin_title();
};


#endif
