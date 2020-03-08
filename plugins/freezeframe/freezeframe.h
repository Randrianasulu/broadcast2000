#ifndef FREEZEFRAME_H
#define FREEZEFRAME_H

// the simplest plugin possible

class FreezeFrameMain;

#include "bcbase.h"
#include "pluginvclient.h"


class FreezeFrameMain : public PluginVClient
{
public:
	FreezeFrameMain(int argc, char *argv[]);
	~FreezeFrameMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	char* plugin_title();
	int start_realtime();
	int stop_realtime();
	int start_gui();
	int stop_gui();
	int show_gui();
	int hide_gui();
	int set_string();
	int save_data(char *text);
	int read_data(char *text);

// parameters needed for freezeframe
	VFrame *first_frame;

private:
// Utilities used by freezeframe.
};


#endif
