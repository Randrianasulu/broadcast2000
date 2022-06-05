#ifndef TIMEAVG_H
#define TIMEAVG_H

class TimeAvgMain;

#include "bcbase.h"
#include "timeavgwindow.h"
#include "pluginvclient.h"


class TimeAvgMain : public PluginVClient
{
public:
	TimeAvgMain(int argc, char *argv[]);
	~TimeAvgMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	const char* plugin_title();
	int start_realtime();
	int stop_realtime();
	int start_gui();
	int stop_gui();
	int show_gui();
	int hide_gui();
	int set_string();
	int load_defaults();
	int save_defaults();
	int save_data(char *text);
	int read_data(char *text);
	int redo_buffers_procedure();

// parameters needed for frame average
	int total_frames;
	int frames_allocated;
	int current_frame;
	int redo_buffers;
	VFrame **ring_buffer;

// a thread for the GUI
	TimeAvgThread *thread;

private:
// Used by frame average
	int test_clip(int &r, int &g, int &b, int &a);
	Defaults *defaults;
};


#endif
