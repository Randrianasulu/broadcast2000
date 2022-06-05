#ifndef NORMALIZE_H
#define NORMALIZE_H

#include "bcbase.h"
#include "pluginaclient.h"

class NormalizeMain : public PluginAClient
{
public:
	NormalizeMain(int argc, char *argv[]);
	~NormalizeMain();

// normalizing engine

// parameters needed

	float db_over;
	int separate_tracks;

// required for all non realtime/multichannel plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int start_plugin();

	int load_defaults();  
	int save_defaults();  
	int get_parameters(); 

	Defaults *defaults;
	int output_rate;
	const char* plugin_title();
	int project_sample_rate;
};


#endif
