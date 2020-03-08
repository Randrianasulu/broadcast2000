#ifndef SHARPEN_H
#define SHARPEN_H

// the simplest plugin possible
// Sharpen leaves the last line too bright

class SharpenMain;
#define MAXSHARPNESS 100

#include "bcbase.h"
#include "sharpenwindow.h"
#include "pluginvclient.h"

class SharpenEngine;

class SharpenMain : public PluginVClient
{
public:
	SharpenMain(int argc, char *argv[]);
	~SharpenMain();

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

// parameters needed for sharpness
	float sharpness; // Range from 0 to 100
	float last_sharpness;
	int interlace;
	int row_step;

// a thread for the GUI
	SharpenThread *thread;
	int pos_lut[VMAX + 1],  neg_lut[VMAX + 1];

private:
	int get_luts(int *pos_lut, int *neg_lut);
	Defaults *defaults;
	SharpenEngine **engine;
	int total_engines;
};


class SharpenEngine : public Thread
{
public:
	SharpenEngine(SharpenMain *plugin);
	~SharpenEngine();

	int start_process_frame(VFrame *output, VFrame *input, int field);
	int wait_process_frame();
	void run();

	int filter(int w, VPixel *src, VPixel *dst, int *neg0, int *neg1, int *neg2);

	SharpenMain *plugin;
	int field;
	VFrame *output, *input;
	int last_frame;
	Mutex input_lock, output_lock;
	VPixel *src_rows[4], *dst_row;
	int *neg_rows[4];
};

#endif
