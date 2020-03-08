#ifndef COMPRESS_H
#define COMPRESS_H

class Compress;

#include "bcbase.h"
#include "compresswindow.h"
#include "pluginaclient.h"

//                                                            -----------attack length-------------
//                      ---------read ahead length------------
//                      data_out                                data_in
//                      *********|||||||||||||||||||||||||||||*********
//        --------------------------------waveform-------------------------------


class Compress : public PluginAClient
{
public:
	Compress(int argc, char *argv[]);
	~Compress();

	int update_gui();
	int load_from_file(char *path);
	int save_to_file(char *path);
	
// data for compress
	float input_level[1024];
	float output_level[1024];
	float input_power[1024];
	float output_power[1024];
	long total_points;
	long last_point;

	long readahead;
	long attack;
	int channel;
	char config_directory[1024];

	long readahead_samples;
	long attack_samples;
	long dsp_length;
	
	long peak_sample1, peak_sample2;
	float peak_value1, peak_value2;
	float current_slope;
	float current_output1, current_power1, current_output2, current_power2;
	float current_output_slope;
	float first_slope, last_slope;

	float *coefficients;
	float **dsp_in;
	long dsp_in_length;
	int redo_buffers;
	DB db;
	
	float get_slope(long sample1, float value1, long sample2, float value2);
	float get_slope(float power1, float scale1, float power2, float scale2);
	int redo_buffers_procedure();
	int import_data(int channel, long size, float *input_ptr);
	int negotiate_coefficients(int channel, long size);
	float get_current_value(long sample);
	float get_coefficient(float input);
	int export_data(int channel, long size, float *output_ptr);
	

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
	
	CompressThread *thread;
};


#endif
