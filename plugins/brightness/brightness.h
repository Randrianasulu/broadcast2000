#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

class BrightnessMain;

#include "bcbase.h"
#include "brightwindow.h"
#include "pluginvclient.h"


class BrightnessMain : public PluginVClient
{
public:
	BrightnessMain(int argc, char *argv[]);
	~BrightnessMain();

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

// parameters needed for brightness
	float brightness;
	float contrast;

// a thread for the GUI
	BrightThread *thread;

private:
// Used by the brightness plugin
	int test_clip(int &r, int &g, int &b);
	Defaults *defaults;
};


#endif
