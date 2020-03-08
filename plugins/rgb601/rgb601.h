#ifndef RGB601_H
#define RGB601_H

// the simplest plugin possible

class RGB601Main;

#include "bcbase.h"
#include "rgb601window.h"
#include "pluginvclient.h"


class RGB601Main : public PluginVClient
{
public:
	RGB601Main(int argc, char *argv[]);
	~RGB601Main();

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
	int load_defaults();
	int save_defaults();
	int cleanup_gui();

	int rgb_to_601, _601_to_rgb;

// a thread for the GUI
	RGB601Thread *thread;

private:
	VWORD rgb_to_601_table[VMAX + 1];
	VWORD _601_to_rgb_table[VMAX + 1];
	Defaults *defaults;
};


#endif
