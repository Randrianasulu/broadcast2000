#ifndef CONVOLVE_H
#define CONVOLVE_H

class Convolve;

#include "bcbase.h"
#include "convolvewindow.h"
#include "pluginaclient.h"

#define WINDOWBORDER (window_size / 2)
#define MAXOFFSET (-INFINITYGAIN)

class CrossfadeWindow
{
public:
	CrossfadeWindow();
	~CrossfadeWindow();

// Process all windows in the input
	int process_fifo(long size, int new_channels, float **output_ptr, float **input_ptr);
	virtual int signal_process(long size, int channels, double **output, double **input) {return 0;};
	int reconfigure(int channels);
	int fix_window_size();
	int init_crossfade(long fragment_size);
	long get_delay();     // Number of samples fifo is delayed

	long window_size;   // Size of a window.  Automatically fixed to a power of 2
	int fragment_size;
	long input_size;      // size of the input_buffer and output_buffer
	long fragment_in_position;  // where in input_buffer to store next incomming fragment
	long fragment_out_position; // where in output_buffer to get next outgoing fragment
	long window_position; // where in input_buffer and output_buffer the current window starts

private:
	int reset_crossfade();
	int delete_crossfade();
	int first_window;   // Don't crossfade the first window
	int allocated_channels;
	float **input_buffer;   // input for complete windows
	float **output_buffer;  // output for crossfaded windows
	double **dsp_in;       // input buffer for signal processor / single window
	double **dsp_out;      // output buffer for signal processor / single window
};

class ConvolveEngine;

class ConvolveSegment : public Thread
{
public:
	ConvolveSegment(ConvolveEngine *engine);
	~ConvolveSegment();

	int start_thread();
	int process_range(long start, long end);
	int wait_process();
	void run();

	inline double dot_product(double *channel1, double level1, double *channel2, double level2, int len)
	{
		register int i;
		register double sum = 0;

		for(i = 0; i < len; i++)
			sum += (*channel1++ * level1) * (*channel2-- * level2);

		return sum;
	};

	long buffer_start, buffer_end;
	int done;
	Mutex input_lock, output_lock;
	ConvolveEngine *engine;
};

class ConvolveEngine : public CrossfadeWindow
{
public:
	ConvolveEngine();
	~ConvolveEngine();

	int signal_process(long size, int channels, double **output, double **input);
	int set_plugin(Convolve *plugin);

	long position;
	Convolve *plugin;
	double *temp;
	long temp_size;
	ConvolveSegment **threads;
	double chan_level[2];
	double **output, **input;
	int total_threads;
};


class Convolve : public PluginAClient
{
public:
	Convolve(int argc, char *argv[]);
	~Convolve();

// procedures

	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int reset();
	int redo_buffers_procedure();


// data
	float chan_level[2];     // frequency to shift
	int automated_level[2];     // Selected automation
	int redo_buffers;         // redo buffers before next render

// required for all multi channel/realtime plugins

	int process_realtime(long size, float **input_ptr, float **output_ptr);
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
	ConvolveThread *thread;          // for the GUI which is dynamically allocated only when needed
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
	ConvolveEngine engine;
};

#endif
