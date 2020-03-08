#ifndef BANDSLIDE_H
#define BANDSLIDE_H

// the simplest plugin possible

class BandSlideMain;

#include "bcbase.h"
#include "bandslidewin.h"
#include "pluginvclient.h"


class BandSlideMain : public PluginVClient
{
public:
	BandSlideMain(int argc, char *argv[]);
	~BandSlideMain();

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

// parameters needed for invert
	int total_bands, reverse;

// a thread for the GUI
	BandSlideThread *thread;

private:
// Utilities used by the processing.
	VFrame *fake_input;
};


#endif
