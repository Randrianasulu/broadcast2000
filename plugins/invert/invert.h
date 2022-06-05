#ifndef LEVEL_H
#define LEVEL_H

// the simplest plugin possible

class InvertMain;

#include "bcbase.h"
#include "invertwindow.h"
#include "pluginaclient.h"


class InvertMain : public PluginAClient
{
public:
	InvertMain(int argc, char *argv[]);
	~InvertMain();

// parameters needed
	int invert;

// required for all realtime plugins
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
	InvertThread *thread;
};


#endif
