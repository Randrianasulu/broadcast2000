#ifndef SCALE_H
#define SCALE_H

// the simplest plugin possible

class ScaleMain;

#include "bcbase.h"
#include "scalewin.h"
#include "overlayframe.h"
#include "pluginvclient.h"


class ScaleMain : public PluginVClient
{
public:
	ScaleMain(int argc, char *argv[]);
	~ScaleMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	const char* plugin_title();
	int start_realtime();
	int stop_realtime();
	int start_gui();
	int stop_gui();
	int cleanup_gui();
	int show_gui();
	int hide_gui();
	int set_string();
	int load_defaults();
	int save_defaults();
	int save_data(char *text);
	int read_data(char *text);

// parameters needed for slide
	float scale_w, scale_h;
	int constrain;

// a thread for the GUI
	ScaleThread *thread;

private:
	OverlayFrame *overlayer;   // To scale images
	VFrame *temp_frame;        // Used if buffers are the same
	Defaults *defaults;
};


#endif
