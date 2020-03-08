#ifndef FRAMERATE_H
#define FRAMERATE_H

#include "bcbase.h"
#include "pluginvclient.h"

class FrameRateMain : public PluginVClient
{
public:
	FrameRateMain(int argc, char *argv[]);
	~FrameRateMain();

// Samplerate parameters

	float *new_rate;

// required for all non realtime/single channel plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int load_defaults();
	int save_defaults();
	int start_plugin();
	float get_plugin_framerate();

	int get_parameters(); 
	char* plugin_title();

	Defaults *defaults;
	float output_rate;
	float input_rate;
};


#endif
