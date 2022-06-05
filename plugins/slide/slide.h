#ifndef SLIDE_H
#define SLIDE_H

// the simplest plugin possible

class SlideMain;

#include "bcbase.h"
#include "slidewin.h"
#include "pluginvclient.h"


class SlideMain : public PluginVClient
{
public:
	SlideMain(int argc, char *argv[]);
	~SlideMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr);
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

// parameters needed for slide
	int reverse;
	int direction;     // 0 - from left    1 - from right

// a thread for the GUI
	SlideThread *thread;

private:
// Utilities used by the processing.
	VFrame *fake_input;
};


#endif
