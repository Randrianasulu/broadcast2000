#ifndef DELAY_H
#define DELAY_H

// the simplest plugin possible

class DelayMain;

#include "bcbase.h"
#include "delaywindow.h"
#include "pluginaclient.h"


class DelayMain : public PluginAClient
{
public:
	DelayMain(int argc, char *argv[]);
	~DelayMain();

// parameters needed by plugin
	long duration;
	float *dsp_in;
	int redo_buffers;
	long dsp_length;

// required for all realtime/single channel plugins
	int process_realtime(long size, float *input_ptr, float *output_ptr);
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
	int save_data(char *text);
	int read_data(char *text);

// a thread for the GUI
	DelayThread *thread;
};


#endif
