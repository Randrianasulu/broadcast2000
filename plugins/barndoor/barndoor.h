#ifndef BARNDOOR_H
#define BARNDOOR_H

// the simplest plugin possible

class BarnDoorMain;

#include "bcbase.h"
#include "barndoorwin.h"
#include "pluginvclient.h"


class BarnDoorMain : public PluginVClient
{
public:
	BarnDoorMain(int argc, char *argv[]);
	~BarnDoorMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr);
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

// parameters needed for slide
	int reverse;

// a thread for the GUI
	BarnDoorThread *thread;

private:
// Utilities used by the processing.
	VFrame *fake_input;
};


#endif
