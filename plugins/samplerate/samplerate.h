#ifndef SAMPLERATE_H
#define SAMPLERATE_H

#include "bcbase.h"
#include "pluginaclient.h"

#define sqr(a)	((a)*(a))

class SampleRateMain : public PluginAClient
{
public:
	SampleRateMain(int argc, char *argv[]);
	~SampleRateMain();

// resampling engine
	double sinc(double x);

	double interpolate_function(double t, double filter_cutoff_freq, double window_key_freq);

	double get_coefficient(int i, int q, 
						 int filter_length, 
						 double filter_cutoff_freq, 
						 double window_key_freq, 
						 double input_freq, 
						 double upsampling_factor, 
						 int downsampling_factor, 
						 double gain);
	int build_coeffient_array();
	int largest_factor(int x, int y);
	
	int transfer_buffer(double *in, double *out, int len);

	int apply_filter(double *dsp_in, double *coefficient_array, int filter_length, double *dsp_out);

	int process_buffer(double *dsp_in, 
								 int input_size, 
								 double *dsp_out,
								 int output_size,
								 double *coefficient_array,
								 int filter_length,
								 int upsampling_factor,
								 int downsampling_factor);

// parameters needed

	int filter_length;
	int upsampling_factor, downsampling_factor;
	double input_freq, window_key_freq, filter_cutoff_freq, gain;
	int in_start, in_offset, cycle_counter, out_start;

// data

	double *coefficient_array;

	int write_dsp(double *dsp_out, long out_buffer_size);





// required for all plugins

	int run_client();
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int get_plugin_samplerate();
	long get_in_buffers(long recommended_size);
	long get_out_buffers(long recommended_size);
	int start_plugin();

	int load_defaults();  
	int save_defaults();  
	int get_parameters(); 

	Defaults *defaults;
	int output_rate;
	const char* plugin_title();
	int input_sample_rate;
	float *buffer_in, *buffer_out;
};


#endif
