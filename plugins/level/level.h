#ifndef LEVEL_H
#define LEVEL_H

// the simplest plugin possible

#define MAXLEVEL 40
class LevelMain;

#include "bcbase.h"
#include "levelwindow.h"
#include "pluginaclient.h"


class LevelMain : public PluginAClient
{
public:
	LevelMain(int argc, char *argv[]);
	~LevelMain();

// parameters needed
	float level;
	DB db;

// required for all realtime plugins
	int process_realtime(long size, float *input_ptr, float *output_ptr);
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
	int load_defaults();
	int save_defaults();

// a thread for the GUI
	LevelThread *thread;
	Defaults *defaults;
};


#endif
