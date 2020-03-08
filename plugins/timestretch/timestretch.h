#ifndef TIMESTRETCH_H
#define TIMESTRETCH_H

#include "bcbase.h"
#include "../fourier/fourier.h"
#include "pluginaclient.h"

#define sqr(a)	((a)*(a))

class TimeStretch;

class ResampleEngine
{
public:
	ResampleEngine(TimeStretch *timestretch);
	~ResampleEngine();

// Perform the resampling
	int do_resample(float *buffer_in, 
				float *buffer_out, 
				long buffer_size, 
				long start,
				long end);
// Functions return 1 to terminate
	virtual int read_buffer(long position, long size) {};
	virtual int write_buffer(long size) {};

	long process_buffer(double *dsp_in, 
							 int input_size, 
							 double *dsp_out,
							 int output_size,
							 double *coefficient_array,
							 int filter_length,
							 int upsampling_factor,
							 int downsampling_factor);

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
	int write_dsp(float *buffer_out, double *dsp_out, long out_buffer_size);

// parameters needed
	int filter_length;
	int upsampling_factor, downsampling_factor;
	double input_freq, window_key_freq, filter_cutoff_freq, gain;
	long in_start, in_offset, cycle_counter, out_start;
	long buffer_size, input_rate, output_rate;
	double *coefficient_array;
	double *dsp_in, *dsp_out;
	TimeStretch *timestretch;
};

class PitchEngine : public CrossfadeFFT
{
public:
	PitchEngine(TimeStretch *timestretch);
	~PitchEngine();

	int signal_process();

	TimeStretch *timestretch;
};


class ExpandEngine : public ResampleEngine
{
public:
	ExpandEngine(TimeStretch *timestretch);
	~ExpandEngine();

	int read_buffer(long position, long size);
	int write_buffer(long size);
};

class ShrinkEngine : public ResampleEngine
{
public:
	ShrinkEngine(TimeStretch *timestretch);
	~ShrinkEngine();

	int read_buffer(long position, long size);
	int write_buffer(long size);
};

class TimeStretch : public PluginAClient
{
public:
	TimeStretch(int argc, char *argv[]);
	~TimeStretch();

	float percentage;
	long input_rate;
	long window_size;
	long leader_size;
	float *buffer_in, *buffer_out, *temp_out;
	CrossfadeFFT fft;
	long total_length;
	PitchEngine *pitch;

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
	char* plugin_title();
};


#endif
