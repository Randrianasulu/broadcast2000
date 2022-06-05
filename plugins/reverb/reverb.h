#ifndef REVERB_H
#define REVERB_H

class Reverb;
class ReverbEngine;

#include "bcbase.h"
#include "reverbwindow.h"
#include "pluginaclient.h"


class Reverb : public PluginAClient
{
public:
	Reverb(int argc, char *argv[]);
	~Reverb();

	int update_gui();
	int load_from_file(char *path);
	int save_to_file(char *path);
	
// data for reverb
	float level_init;
	long delay_init;
	float ref_level1;
	float ref_level2;
	long ref_total;
	long ref_length;
	Freq lowpass1, lowpass2;
	char config_directory[1024];

	float **main_in, **main_out;
	double **dsp_in;
	long **ref_channels, **ref_offsets, **ref_lowpass;
	float **ref_levels;
	long dsp_in_length;
	int redo_buffers;
// skirts for lowpass filter
	double **lowpass_in1, **lowpass_in2;
	DB db;
// required for all realtime/multichannel plugins

	int process_realtime(long size, float **input_ptr, float **output_ptr);
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

// non realtime support
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
	
	ReverbThread *thread;
	ReverbEngine **engine;
};

class ReverbEngine : public Thread
{
public:
	ReverbEngine(Reverb *plugin);
	~ReverbEngine();

	int process_overlay(float *in, double *out, double &out1, double &out2, double level, long lowpass, long samplerate, long size);
	int process_overlays(int output_buffer, long size);
	int wait_process_overlays();
	void run();

	Mutex input_lock, output_lock;
	int completed;
	int output_buffer;
	long size;
	Reverb *plugin;
};

#endif
