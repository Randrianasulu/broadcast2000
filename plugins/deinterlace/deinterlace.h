#ifndef DEINTERLACE_H
#define DEINTERLACE_H

// the simplest plugin possible

class DeInterlaceMain;

#include "bcbase.h"
#include "deinterwindow.h"
#include "pluginvclient.h"


class DeInterlaceMain : public PluginVClient
{
public:
	DeInterlaceMain(int argc, char *argv[]);
	~DeInterlaceMain();

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

// parameters needed for invert
	int odd_fields, even_fields, average_fields, swap_fields, smart_fields;

// a thread for the GUI
	DeInterlaceThread *thread;
	VFrame *previous, *test_frame;

private:
// Utilities used by deinterlace.
	int copy_row(VPixel *input_row, VPixel *output_row, int w);
	int average_row(VPixel *output1, VPixel *output2, VPixel *input1, VPixel *input2, int w);
	int swap_row(VPixel *output1, VPixel *output2, VPixel *input1, VPixel *input2, int w);
	void inverse_telecine(VFrame *input_ptr, VFrame *output_ptr);
	void copy_field(int field, VFrame *input_ptr, VFrame *output_ptr);
};


#endif
