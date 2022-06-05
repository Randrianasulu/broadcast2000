#ifndef SHIFTINTERLACE_H
#define SHIFTINTERLACE_H

class ShiftInterlaceMain;

#include "bcbase.h"
#include "shiftwindow.h"
#include "pluginvclient.h"


class ShiftInterlaceMain : public PluginVClient
{
public:
	ShiftInterlaceMain(int argc, char *argv[]);
	~ShiftInterlaceMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);
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
	int load_defaults();
	int save_defaults();

	int odd_offset, even_offset;

private:
	int shift_row(VPixel *output_row, VPixel *input_row, int offset);
	ShiftThread *thread;
	Defaults *defaults;
};


#endif
