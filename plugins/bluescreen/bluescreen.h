#ifndef BLUESCREEN_H
#define BLUESCREEN_H

class BluescreenMain;
class BluescreenEngine;

#include "bcbase.h"
#include "../colors/colors.h"
#include "bluewindow.h"
#include "pluginvclient.h"


class BluescreenMain : public PluginVClient
{
public:
	BluescreenMain(int argc, char *argv[]);
	~BluescreenMain();

// Procedures for the bluescreen
	int update_rgb();
	int set_hue(float value);
	int set_saturation(float value);
	int set_value(float value);
	int set_red(float value);
	int set_green(float value);
	int set_blue(float value);
	int update_display();    // Updates the GUI with a new color
	int rgb_to_hsv(float r, float g, float b, float &h, float &s, float &v);
	int hsv_to_rgb(float &r, float &g, float &b, float h, float s, float v);

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

// parameters needed for bluescreen
// Floats are used to make values independant of word size
	float h, s, v;    // h ranges from 0 to 360
	                  // s ranges from 0 to 1
					  // v ranges from 0 to 1
	float r, g, b;      // All range from 0 to 1
	float threshold;  // threshold ranges from 0 to 255
	float feather;    // feather radius in hue

private:
// a thread for the GUI
	BlueThread *thread;
	Defaults *defaults;
	BluescreenEngine **engine;
};


class BluescreenEngine : public Thread
{
public:
	BluescreenEngine(BluescreenMain *plugin, int start_y, int end_y);
	~BluescreenEngine();
	
	int start_process_frame(VFrame **output, VFrame **input, int size);
	int wait_process_frame();
	void run();

	BluescreenMain *plugin;
	int start_y;
	int end_y;
	int size;
	VFrame **output, **input;
	int last_frame;
	Mutex input_lock, output_lock;
    HSV hsv;
};

#endif
