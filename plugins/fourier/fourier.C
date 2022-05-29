#include <math.h>
#include "fourier.h"
#include "filehtal.h"


FFT::FFT()
{
}

FFT::~FFT()
{
}

int FFT::do_fft(unsigned int samples,  // must be a power of 2
    	int inverse,         // 0 = forward FFT, 1 = inverse
    	double *real_in,     // array of input's real samples
    	double *imag_in,     // array of input's imag samples
    	double *real_out,    // array of output's reals
    	double *imag_out)
{
    unsigned int num_bits;    // Number of bits needed to store indices
    register unsigned int i, j, k, n;
    unsigned int block_size, block_end;

    double angle_numerator = 2.0 * M_PI;
    double tr, ti;     // temp real, temp imaginary

    if(inverse)
        angle_numerator = -angle_numerator;

    num_bits = samples_to_bits(samples);

// Do simultaneous data copy and bit-reversal ordering into outputs

    for(i = 0; i < samples; i++)
    {
        j = reverse_bits(i, num_bits);
        real_out[j] = real_in[i];
        imag_out[j] = (imag_in == NULL) ? 0.0 : imag_in[i];
    }

// Do the FFT itself

    block_end = 1;
    double delta_angle;
    double sm2;
    double sm1;
    double cm2;
    double cm1;
    double w;
    double ar[3], ai[3];
    double temp;
    for(block_size = 2; block_size <= samples; block_size <<= 1)
    {
        delta_angle = angle_numerator / (double)block_size;
        sm2 = sin(-2 * delta_angle);
        sm1 = sin(-delta_angle);
        cm2 = cos(-2 * delta_angle);
        cm1 = cos(-delta_angle);
        w = 2 * cm1;

        for(i = 0; i < samples; i += block_size)
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for(j = i, n = 0; n < block_end; j++, n++)
            {
                ar[0] = w * ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w * ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + block_end;
                tr = ar[0] * real_out[k] - ai[0] * imag_out[k];
                ti = ar[0] * imag_out[k] + ai[0] * real_out[k];

                real_out[k] = real_out[j] - tr;
                imag_out[k] = imag_out[j] - ti;

                real_out[j] += tr;
                imag_out[j] += ti;
            }
        }

        block_end = block_size;
    }

// Normalize if inverse transform

    if(inverse)
    {
        double denom = (double)samples;

        for (i = 0; i < samples; i++)
        {
            real_out[i] /= denom;
            imag_out[i] /= denom;
        }
    }
	return 0;
}


unsigned int FFT::samples_to_bits(unsigned int samples)
{
    register unsigned int i;

    for(i = 0; ; i++)
    {
        if(samples & (1 << i))
            return i;
    }
	return i;
}

unsigned int FFT::reverse_bits(unsigned int index, unsigned int bits)
{
    register unsigned int i, rev;

    for(i = rev = 0; i < bits; i++)
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }

    return rev;
}

int FFT::symmetry(int size, double *freq_real, double *freq_imag)
{
    int h = size / 2;
    for(register int i = h + 1; i < size; i++)
    {
        freq_real[i] = freq_real[size - i];
        freq_imag[i] = -freq_imag[size - i];
    }
	return 0;
}





CrossfadeFFT::CrossfadeFFT() : FFT()
{
	reset_fourier();
}

CrossfadeFFT::~CrossfadeFFT()
{
	delete_fourier();
}

int CrossfadeFFT::reset_fourier()
{
	window_size = 4096;
return 0;
}

int CrossfadeFFT::delete_fourier()
{
	if(input_buffer)
	{
		delete dsp_in;
		delete dsp_out;
		delete input_buffer;
		delete output_buffer;
		delete freq_real;
		delete freq_imag;
		delete temp_imag;
	}
	input_buffer = 0;
return 0;
}

int CrossfadeFFT::fix_window_size()
{
// fix the window size
// window size must be a power of 2
	int new_size = 16;
	while(new_size < window_size) new_size *= 2;
	window_size = new_size;
return 0;
}

int CrossfadeFFT::init_fft(int fragment_size)
{
	this->fragment_size = fragment_size;
	first_window = 1;
return 0;
}

long CrossfadeFFT::get_delay()
{
	return window_size + WINDOWBORDER;
return 0;
}

int CrossfadeFFT::reconfigure()
{
	int new_size;
	fix_window_size();

	if(first_window)
	{
		fragment_in_position = 0;
		window_position = 0;
		fragment_out_position = -get_delay();
		input_buffer = 0;
		input_size = 0;
	}

// get the dsp buffer size to be allocated
// only change it if it's bigger
	new_size = window_size + fragment_size * 2 + WINDOWBORDER;
	if(new_size <= input_size) new_size = input_size;
	float *new_input_buffer = new float[new_size];
	float *new_output_buffer = new float[new_size];

// copy data from the old input_buffer and clear the new buffer
	register int i = 0;
	if(input_buffer)
	{
		for(; i < input_size; i++)
		{
			new_input_buffer[i] = input_buffer[i];
			new_output_buffer[i] = output_buffer[i];
		}
	}

	while(i < new_size)
	{
		new_input_buffer[i] = 0;
		new_output_buffer[i] = 0;
		i++;
	}

// Changing the window size greater than the fragment size will
// cause glitches since the fragment delay isn't adjusted here.
	input_size = new_size;
	delete_fourier();

	dsp_in = new double[window_size];
	dsp_out = new double[window_size];
	freq_real = new double[window_size];
	freq_imag = new double[window_size];
	temp_imag = new double[window_size];
	input_buffer = new_input_buffer;
	output_buffer = new_output_buffer;
return 0;
}

int CrossfadeFFT::process_fifo(long size, float *input_ptr, float *output_ptr)
{
	float *input_buffer_end = input_buffer + input_size;
	float *output_buffer_end = output_buffer + input_size;

// copy the new fragment into the input_buffer
	{
		register float *output = &input_buffer[fragment_in_position];
		register float *output_end = output + size;
		if(output_end >= input_buffer_end) output_end -= input_size;

		while(output != output_end)
		{
			*output++ = *input_ptr++;
			if(output >= input_buffer_end) output = input_buffer;
		}
	}

	fragment_in_position += size;
	if(fragment_in_position >= input_size) fragment_in_position -= input_size;

// have enough to process a window
	while((fragment_in_position < window_position && input_size - window_position + fragment_in_position >= window_size) ||
		(fragment_in_position - window_position >= window_size))
	{
// copy a window from input_buffer to the dsp_in
		{
			register float *input = &input_buffer[window_position];
			register float *input_end = input + window_size;
			if(input_end > input_buffer_end) input_end -= input_size;
			double *output = dsp_in;

			while(input != input_end)
			{
				if(input >= input_buffer_end) input = input_buffer;
				*output++ = *input++;
			}
		}

		do_fft(window_size,  // must be a power of 2
    		0,         // 0 = forward FFT, 1 = inverse
    		dsp_in,     // array of input's real samples
    		0,     // array of input's imag samples
    		freq_real,    // array of output's reals
    		freq_imag);

		signal_process();

		do_fft(window_size,  // must be a power of 2
    		1,         // 0 = forward FFT, 1 = inverse
    		freq_real,     // array of input's real samples
    		freq_imag,     // array of input's imag samples
    		dsp_out,    // array of output's reals
    		temp_imag);

// copy from dsp_out into the output_buffer
		{
// crossfade first WINDOWBORDER with last WINDOWBORDER of previous window
			register float *output = &output_buffer[window_position];
			register float *output_end = output + WINDOWBORDER;
			register double *input = dsp_out;
			double input_slope = (double)1 / WINDOWBORDER;
			float output_slope = (double)1 / WINDOWBORDER;
			double scale = 0;
			if(output_end > output_buffer_end) output_end -= input_size; 

// Don't crossfade first window
			if(first_window)
				while(output != output_end)
				{
					if(output >= output_buffer_end) output = output_buffer;
					*output++ = *input++;
				}
			else
				while(output != output_end)
				{
					if(output >= output_buffer_end) output = output_buffer;
					*input *= scale / WINDOWBORDER;
					*output *= 1 - scale / WINDOWBORDER;
					*output++ += *input++;
					scale++;
				}

			first_window = 0;
// overwrite rest of output with input
			output_end = output + window_size - WINDOWBORDER;
			if(output_end > output_buffer_end) output_end -= input_size; 

			while(output != output_end)
			{
				if(output >= output_buffer_end) output = output_buffer;
				*output++ = *input++;
			}
		}

// advance the window position
		window_position += window_size - WINDOWBORDER;
		if(window_position >= input_size) window_position -= input_size;
	}


// write the fragment from the oldest processed window to output
	float *output_end = output_ptr + size;

// Initially the fragment_out_position is < 0
	if(fragment_out_position < 0)
	{
		register float *new_output_end = output_end;
		if(fragment_out_position + size > 0) new_output_end -= fragment_out_position + size;
		while(output_ptr < new_output_end)
		{
			*output_ptr++ = 0;
		}
	}

	float *input;
	if(fragment_out_position < 0) 
		input = output_buffer;
	else
		input = &output_buffer[fragment_out_position];

	while(output_ptr < output_end)
	{
		*output_ptr++ = *input++;
		if(input >= output_buffer_end) input = output_buffer;
	}

	fragment_out_position += size;
	if(fragment_out_position >= input_size) fragment_out_position -= input_size;
return 0;
}






SlidingFFT::SlidingFFT()
{
	dsp_buffer = 0;
	dsp_out = 0;
	window_size = 4096;
	fragment_size = 0;
	dsp_length = 0;
	freq_real = freq_imag = temp_imag = 0;
}

SlidingFFT::~SlidingFFT()
{
	if(dsp_buffer) delete dsp_buffer;
	if(dsp_out) delete dsp_out;
	if(freq_real) delete freq_real;
	if(freq_imag) delete freq_imag;
	if(temp_imag) delete temp_imag;
}

int SlidingFFT::init_fft(int fragment_size)
{
	this->fragment_size = fragment_size;
return 0;
}

int SlidingFFT::reconfigure()
{
	int i, new_window, new_length;
	double *new_buffer;

	if(dsp_length != fragment_size + window_size)
	{
		for(new_window = 16; new_window < window_size; new_window *= 2)
			;

		new_length = new_window + fragment_size;
		new_buffer = new double[new_length];

		for(i = 0; i < dsp_length && i < new_length; i++)
		{
			new_buffer[i] = dsp_buffer[i];
		}
		
		for( ; i < new_length; i++)
		{
			new_buffer[i] = 0;
		}

		if(freq_real) delete freq_real;
		if(freq_imag) delete freq_imag;
		if(temp_imag) delete temp_imag;

		freq_real = new double[new_window];
		freq_imag = new double[new_window];
		temp_imag = new double[new_window];

		if(dsp_buffer) delete dsp_buffer;
		dsp_buffer = new_buffer;
		dsp_length = new_length;
		window_size = new_window;
	}
	
	if(!dsp_out) dsp_out = new double[fragment_size];
return 0;
}

int SlidingFFT::process_fifo(long size, float *input_ptr, float *output_ptr)
{
	int i, j, h;
	reconfigure();

// Shift data back by size
	for(i = 0, j = size; j < dsp_length; i++, j++)
		dsp_buffer[i] = dsp_buffer[j];

// Load new data
	for(i = dsp_length - size, j = 0 ; i < dsp_length; i++, j++)
		dsp_buffer[i] = input_ptr[j];

// Process oldest data
	for(i = dsp_length - window_size - size, j = 0;
		i < dsp_length - window_size;
		i++, j++)
	{
		do_fft(window_size,  // must be a power of 2
    		0,         // 0 = forward FFT, 1 = inverse
    		&dsp_buffer[i],     // array of input's real samples
    		0,     // array of input's imag samples
    		freq_real,    // array of output's reals
    		freq_imag);

		signal_process();

		do_fft(window_size,  // must be a power of 2
    		1,         // 0 = forward FFT, 1 = inverse
    		freq_real,     // array of input's real samples
    		freq_imag,     // array of input's imag samples
    		dsp_out,    // array of output's reals
    		temp_imag);
		
		output_ptr[j] = dsp_out[window_size / 2];
	}
	return 0;
}



