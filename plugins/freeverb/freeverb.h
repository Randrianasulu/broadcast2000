#ifndef FREEVERB_H
#define FREEVERB_H

class Freeverb;
class FreeverbEngine;

#include "Components/revmodel.h"
#include "freeverbwin.h"
#include "pluginaclient.h"
#include "units.h"


class Freeverb : public PluginAClient
{
public:
	Freeverb(int argc, char *argv[]);
	~Freeverb();

	int update_gui();
	int load_from_file(char *path);
	int save_to_file(char *path);

	float roomsize;
	float damping;
	float wetness;
	float dryness;
	float gain;
	int redo_buffers;

// required for all realtime/multichannel plugins

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

// non realtime support
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
	
	FreeverbThread *thread;

private:
	DB db;
	float **tmp_output;
	long tmp_size;
	revmodel *engine;
};

class FreeverbEngine : public Thread
{
public:
	FreeverbEngine(Freeverb *plugin);
	~FreeverbEngine();

	int process_overlay(float *in, double *out, double &out1, double &out2, double level, long lowpass, long samplerate, long size);
	int process_overlays(int output_buffer, long size);
	int wait_process_overlays();
	void run();

	Mutex input_lock, output_lock;
	int completed;
	int output_buffer;
	long size;
	Freeverb *plugin;
};

#endif
