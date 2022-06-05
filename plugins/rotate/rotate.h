#ifndef ROTATE_H
#define ROTATE_H

class RotateMain;
#define MAXANGLE 360

#include "bcbase.h"
#include "rotatewindow.h"
#include "pluginvclient.h"

typedef struct
{
	float x, y;
} SourceCoord;

class RotateEngine : public Thread
{
public:
	RotateEngine(RotateMain *plugin, int row1, int row2);
	~RotateEngine();
	
	int generate_matrix(int interpolate);
	int perform_rotation(VPixel **input_rows, VPixel **output_rows, int interpolate);
	int wait_completion();
	int create_matrix();
	int coords_to_pixel(int &input_y, int &input_x);
	int coords_to_pixel(SourceCoord &float_pixel, float &input_y, float &input_x);
	int perform_rotation();
	void run();
	
	int row1, row2;
	int interpolate;
	int do_matrix, do_rotation;
	int done;
	RotateMain *plugin;
	Mutex input_lock, output_lock;
	VPixel **input_rows;
	VPixel **output_rows;
};


class RotateMain : public PluginVClient
{
public:
	RotateMain(int argc, char *argv[]);
	~RotateMain();
	
	friend RotateEngine;

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
	int load_defaults();
	int save_defaults();
	int save_data(char *text);
	int read_data(char *text);

// parameters needed for rotate
	int update_values(float value);
    int rotate_rightangle(VPixel **input_rows, VPixel **output_rows, int angle);
    int rotate_obliqueangle(VPixel **input_rows, VFrame *output, int angle);
    int get_rightdimensions(int &diameter, int &in_x1, int &in_y1, int &in_x2, int &in_y2, int &out_x1, int &out_y1, int &out_x2, int &out_y2);
    int clear_unused(VPixel **input_rows, VPixel **output_rows, int out_x1, int out_y1, int out_x2, int out_y2);
	float angle;

// a thread for the GUI
	RotateThread *thread;
// Threads to parallelize the rotation and matrix generation.
	RotateEngine **engine;

private:
	int test_clip(int &r, int &g, int &b, int &a);
	Defaults *defaults;
// Temporary frame used for oblique angle rotations.
	VFrame *temp_frame;
// Matrix of source pixel offsets
	int *int_matrix, **int_rows;
// Interpolation uses input coordinates for each output coordinate.
	SourceCoord *float_matrix, **float_rows;
// Compare new angle with old angle
	float last_angle;
};


#endif
