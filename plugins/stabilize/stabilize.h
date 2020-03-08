#ifndef STABILIZE_H
#define STABILIZE_H

class StabilizeMain;
#define MAXACCEL 256

#include "bcbase.h"
#include "stabilizewindow.h"
#include "pluginvclient.h"

class StabilizeEngine : public Thread
{
public:
	StabilizeEngine(StabilizeMain *plugin);
	~StabilizeEngine();
	
	int exhaustive_search(int block_x, int block_y, VPixel **block_rows, VPixel **input_rows);
	int wait_completion(long &score, int &x_offset, int &y_offset);
	long compare_blocks(int x, int y, VPixel **block_rows, VPixel **input_rows);
	void run();
	
	int y1, y2;
	int done;
	StabilizeMain *plugin;
	Mutex input_lock, output_lock;
	VPixel **input_rows;
	VPixel **block_rows;
	int block_x, block_y;
	long score;
	int x_offset, y_offset;
};


class StabilizeMain : public PluginVClient
{
public:
	StabilizeMain(int argc, char *argv[]);
	~StabilizeMain();

	int get_vector(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset);
	int read_block(int x, int y, VFrame *block, VFrame *input);
	int exhaustive_search(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset);
	int three_step_search(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset);
	int get_range(int x, int y, 
		int &in_x1, int &in_y1, int &in_x2, int &in_y2,
        int &out_x1, int &out_y1, int &out_x2, int &out_y2);
	int recenter(int &new_x, int &new_y);

// Straight from destabilize
	int offset_frame(VFrame *in, VFrame *out, int x_offset, int y_offset);
	int clear_row(VPixel *row);

// required for all realtime plugins
	int process_realtime(long frames, VFrame **input_ptr, VFrame **output_ptr);
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

// parameters needed for processor
	int range;   // Distance to search for motion vectors
	int size;	// Size of block edge
	int bound;
	int block_x, block_y;    // Position of block read after shifting
	VFrame *block;    // Block in previous frame to find the current position of in the current frame
	int redo_buffers;
	int accel;   // Speed of responsiveness
	int x_offset, y_offset;   // Amount to offset output frame.  Inverse of the shift.
	long block_randomness;    // Amount of utility in the block

// a thread for the GUI
	StabilizeThread *thread;
// Threads to parallelize the searches.
	StabilizeEngine **engine;

private:
	Defaults *defaults;
};


#endif
