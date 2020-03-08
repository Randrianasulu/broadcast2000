#ifndef BLUR_H
#define BLUR_H

class BlurMain;
class BlurEngine;

#define MAXRADIUS 20

#include "bcbase.h"
#include "blurwindow.h"
#include "pluginvclient.h"

typedef struct
{
	float r;
    float g;
    float b;
    float a;
} pixel_f;

class BlurMain : public PluginVClient
{
public:
	BlurMain(int argc, char *argv[]);
	~BlurMain();

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

	int vertical;
	int horizontal;
	int radius;
    int redo_buffers;
	int last_radius;    // Radius used in last frame for automation

// a thread for the GUI
	BlurThread *thread;

private:
	Defaults *defaults;
	BlurEngine **engine;
};


class BlurEngine : public Thread
{
public:
	BlurEngine(BlurMain *plugin, int start_y, int end_y);
	~BlurEngine();

	void run();
	int start_process_frame(VFrame **output, VFrame **input, int size);
	int wait_process_frame();

// parameters needed for blur
	int get_constants();
	int reconfigure();
	int transfer_pixels(pixel_f *src1, pixel_f *src2, pixel_f *dest, int size);
	int multiply_alpha(pixel_f *row, int size);
	int seperate_alpha(pixel_f *row, int size);
	int blur_strip(int &j, int &size);

	pixel_f *val_p, *val_m, *vp, *vm;
	pixel_f *sp_p, *sp_m;
    float n_p[5], n_m[5];
    float d_p[5], d_m[5];
    float bd_p[5], bd_m[5];
    float std_dev;
	pixel_f *src, *dst;
    pixel_f initial_p;
    pixel_f initial_m;
	int terms;
	BlurMain *plugin;
// A margin is introduced between the input and output to give a seemless transition between blurs
	int start_in, start_out;
	int end_in, end_out;
	int size;
	VFrame **output, **input;
	int last_frame;
	Mutex input_lock, output_lock;
};

#endif
