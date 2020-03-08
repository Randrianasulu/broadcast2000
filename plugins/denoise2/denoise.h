#ifndef DENOISE_H
#define DENOISE_H

class Denoise;


#include "bcbase.h"
#include "denoisewindow.h"
#include "pluginaclient.h"

typedef enum { DECOMP, RECON } wavetype;
#define SGN(x) (x<0 ? -1: 1)
#define WINDOWBORDER (window_size / 2)

class Tree
{
public:
	Tree(int input_length, int levels);
	~Tree();
	
	int input_length;
	int levels;
	double **values;
};

class WaveletCoeffs
{
public:
	WaveletCoeffs(double alpha, double beta);
	~WaveletCoeffs();
	
	double values[6];
	int length;
};

class WaveletFilters
{
public:
	WaveletFilters(WaveletCoeffs *wave_coeffs, wavetype transform);
	~WaveletFilters();

	double g[6], h[6];
	int length;
};

class Denoise : public PluginAClient
{
public:
	Denoise(int argc, char *argv[]);
	~Denoise();

// procedures

	update_gui();         // update all the controls with new values
	load_from_file(char *path);
	save_to_file(char *path);
	reset_parameters();
	reset();
	init_buffers();
	redo_buffers_procedure();
	delete_buffers();
	process_window();
	wavelet_decomposition(double *in_data, long in_length, double **out_data);
	long decompose_branches(double *in_data, long length, WaveletFilters *decomp_filter, double *out_low, double *out_high);
	convolve_dec_2(double *input_sequence, long length,
					double *filter, int filtlen, double *output_sequence);
	double dot_product(double *data, double *filter, char filtlen);
	tree_copy(double **output, double **input, int length, int levels);
	threshold(int window_size, double noise_level, int levels);
	wavelet_reconstruction(double **in_data, long in_length, double *out_data);
	long reconstruct_branches(double *in_low, double *in_high, long in_length,
				WaveletFilters *recon_filter, double *output);
	convolve_int_2(double *input_sequence, long length, 
					double *filter, int filtlen, int sum_output, double *output_sequence);
	double dot_product_even(double *data, double *filter, int filtlen);
	double dot_product_odd(double *data, double *filter, int filtlen);


// data for denoiser

	long levels;     // depends on the type of music
	long iterations;    // higher number reduces aliasing due to a high noise_level
						// also increases high end
	float output_level;       // power
	float noise_level;     // higher number kills more noise at the expense of more aliasing
	long window_size;
	float *input_buffer;       // buffer for storing fragments until a complete window size is armed
	float *output_buffer;      // buffer for storing fragments until a fragment is ready to be read
	double *dsp_in;     // buffers for processing windows
	double *dsp_iteration;   // buffer for capturing output of a single iteration
	double *dsp_out;     // buffer for adding iterations
	int redo_buffers;         // redo buffers before next render
	float alpha, beta;          // daub6 coeffs
	double in_scale;    // scaling factor for transferring from input_buffer to dsp_in
	double out_scale;         // power converted to scaling factor
	long input_size;      // size of the input_buffer
	long fragment_in_position;  // where in input_buffer to store next incomming fragment
	long fragment_out_position; // where in output_buffer to get next outgoing fragment
	long window_in_position; // where in input_buffer to read next window

	Tree *ex_coeff_d, *ex_coeff_r, *ex_coeff_rn;
	WaveletCoeffs *wave_coeff_d, *wave_coeff_r;
	WaveletFilters *decomp_filter, *recon_filter;
	double lambda1[9], lambda2[9];

// required for all single channel/realtime plugins

	process_realtime(long size, float *input_ptr, float *output_ptr);
	plugin_is_realtime();
	plugin_is_multi_channel();
	char* plugin_title();
	start_realtime();
	stop_realtime();
	start_gui();
	stop_gui();
	show_gui();
	hide_gui();
	set_string();
	save_data(char *text);
	read_data(char *text);
	DenoiseThread *thread;          // for the GUI which is dynamically allocated only when needed
	load_defaults();
	save_defaults();
	Defaults *defaults;
};

#endif
