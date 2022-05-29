#ifndef FOURIER_H
#define FOURIER_H

#include "defaults.h"
#include "filehtal.inc"
#include "pluginaclient.h"

#define WINDOWBORDER (window_size / 2)

class FFT
{
public:
	FFT();
	~FFT();

	int do_fft(unsigned int samples,  // must be a power of 2
    	int inverse,         // 0 = forward FFT, 1 = inverse
    	double *real_in,     // array of input's real samples
    	double *imag_in,     // array of input's imag samples
    	double *real_out,    // array of output's reals
    	double *imag_out);   // array of output's imaginaries
	int symmetry(int size, double *freq_real, double *freq_imag);
	unsigned int samples_to_bits(unsigned int samples);
	unsigned int reverse_bits(unsigned int index, unsigned int bits);
};

class CrossfadeFFT : public FFT
{
public:
	CrossfadeFFT();
	virtual ~CrossfadeFFT();

	int reset_fourier();
	int init_fft(int fragment_size);
	long get_delay();     // Number of samples fifo is delayed
	int reconfigure();
	int fix_window_size();
	int delete_fourier();
// Process all windows in the input
	int process_fifo(long size, float *input_ptr, float *output_ptr);

	virtual int signal_process() {return 0;};        // Process in the frequency domain

// data for fourier
	long window_size;   // Size of a window.  Automatically fixed to a power of 2

	int fragment_size;
	float *input_buffer;   // input for complete windows
	float *output_buffer;  // output for crossfaded windows
	double *dsp_in;       // input buffer for signal processor / single window
	double *dsp_out;      // output buffer for signal processor / single window

	long input_size;      // size of the input_buffer and output_buffer
	long fragment_in_position;  // where in input_buffer to store next incomming fragment
	long fragment_out_position; // where in output_buffer to get next outgoing fragment
	long window_position; // where in input_buffer and output_buffer the current window starts

// Output of FFT
	double *freq_real;
	double *freq_imag;

	double *temp_imag;

private:
	int first_window;   // Don't crossfade the first window
};

class SlidingFFT : public FFT
{
public:
	SlidingFFT();
	virtual ~SlidingFFT();

	int init_fft(int fragment_size);
	int reconfigure();
	int process_fifo(long size, float *input_ptr, float *output_ptr);
	virtual int signal_process() { return 0; };    // user processing here

	int dsp_length;
	double *dsp_buffer, *dsp_out;
	int window_size;
	int fragment_size;
	double *freq_real;
	double *freq_imag;
	double *temp_imag;
};

#endif
