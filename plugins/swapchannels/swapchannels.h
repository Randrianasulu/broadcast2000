#ifndef SWAPCHANNELS_H
#define SWAPCHANNELS_H

class SwapMain;

#define RED_SRC 0
#define GREEN_SRC 1
#define BLUE_SRC 2
#define ALPHA_SRC 3
#define NO_SRC 4
#define MAX_SRC 5

#include "bcbase.h"
#include "swapwindow.h"
#include "pluginvclient.h"

class SwapMain : public PluginVClient
{
public:
	SwapMain(int argc, char *argv[]);
	~SwapMain();

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
	int load_defaults();
	int save_defaults();
	int save_data(char *text);
	int read_data(char *text);

// parameters needed for processor
	char* output_to_text(int value);
	int text_to_output(char *text);
	int red;
	int green;
	int blue;
	int alpha;

private:
	SwapThread *thread;
	Defaults *defaults;
};


#endif
