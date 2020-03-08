#ifndef CDRIPPER_H
#define CDRIPPER_H

#include "bcbase.h"
#include "pluginaclient.h"

#include <linux/cdrom.h>

#define NFRAMES    2
#define FRAMESIZE  2352


class CDRipMain : public PluginAClient
{
public:
	CDRipMain(int argc, char *argv[]);
	~CDRipMain();


// parameters needed
	int track1, min1, sec1, track2, min2, sec2;
	char device[1024];
	
	long startlba, endlba;
	int cdrom;
	int get_toc();
	int open_drive();
	int close_drive();

// required for all non realtime plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int get_plugin_samplerate();
	long get_in_buffers(long recommended_size);
	long get_out_buffers(long recommended_size);
	int start_plugin();

	int load_defaults();  
	int save_defaults();  
	int get_parameters(); 

	Defaults *defaults;
	int output_rate;
	char* plugin_title();
};


#endif
