#ifndef WIPE_H
#define WIPE_H

// the simplest plugin possible

class IrisSquareMain;

#include "bcbase.h"
#include "irissquarewin.h"
#include "pluginvclient.h"


class IrisSquareMain : public PluginVClient
{
public:
	IrisSquareMain(int argc, char *argv[]);
	~IrisSquareMain();

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

// parameters needed for irissquare
	int reverse;

// a thread for the GUI
	IrisSquareThread *thread;

private:
// Utilities used by the processing.
	VFrame *fake_input;
};


#endif
