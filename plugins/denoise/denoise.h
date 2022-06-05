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

	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int reset();
	int init_buffers();
	int redo_buffers_procedure();
	int delete_buffers();
	int process_window();
	int wavelet_decomposition(double *in_data, long in_length, double **out_data);
	int long decompose_branches(double *in_data, long length, WaveletFilters *decomp_filter, double *out_low, double *out_high);
	int convolve_dec_2(double *input_sequence, long length,
					double *filter, int filtlen, double *output_sequence);
	double dot_product(double *data, double *filter, char filtlen);
	int tree_copy(double **output, double **input, int length, int levels);
	int threshold(int window_size, double gammas, int levels);
	int wavelet_reconstruction(double **in_data, long in_length, double *out_data);
	long reconstruct_branches(double *in_low, double *in_high, long in_length,
				WaveletFilters *recon_filter, double *output);
	int convolve_int_2(double *input_sequence, long length, 
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
	long window_position; // where in input_buffer to read next window
	int first_window;

	Tree *ex_coeff_d, *ex_coeff_r, *ex_coeff_rn;
	WaveletCoeffs *wave_coeff_d, *wave_coeff_r;
	WaveletFilters *decomp_filter, *recon_filter;

// required for all single channel/realtime plugins

	int process_realtime(long size, float *input_ptr, float *output_ptr);
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
	DenoiseThread *thread;          // for the GUI which is dynamically allocated only when needed
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
};

#endif
