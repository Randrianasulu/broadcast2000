#ifndef FLIP_H
#define FLIP_H

// the simplest plugin possible

class FlipMain;

#include "bcbase.h"
#include "flipwindow.h"
#include "pluginvclient.h"


class FlipMain : public PluginVClient
{
public:
	FlipMain(int argc, char *argv[]);
	~FlipMain();

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

// parameters needed for flip
	int flip_horizontal, flip_vertical;

// a thread for the GUI
	FlipThread *thread;

private:
// Utilities used by flip.
	int swap_pixels(VPixel *in, VPixel *out);
};


#endif
