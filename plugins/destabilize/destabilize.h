#ifndef DESTABILIZE_H
#define DESTABILIZE_H

class DestabilizeMain;

#include "bcbase.h"
#include "destabilizewin.h"
#include "pluginvclient.h"


class DestabilizeMain : public PluginVClient
{
public:
	DestabilizeMain(int argc, char *argv[]);
	~DestabilizeMain();

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

// parameters needed for destabilize
	int get_coordinate(int &x, int x1, int x2);
	int advance_position();
	int offset_frame(VFrame *in, VFrame *out, int x_offset, int y_offset);
	int clear_row(VPixel *row);
	int range, accel, speed;
	float current_position;
	int x_offset, x_offset1, x_offset2;
	int y_offset, y_offset1, y_offset2;

// a thread for the GUI
	DestabilizeThread *thread;

private:
	Defaults *defaults;
};


#endif
