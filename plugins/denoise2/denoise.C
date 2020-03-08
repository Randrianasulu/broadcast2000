#include <math.h>
#include "denoise.h"
#include "filehtal.h"

main(int argc, char *argv[])
{
	Denoise *plugin;

	plugin = new Denoise(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

Tree::Tree(int input_length, int levels)
{
	this->input_length = input_length;
	this->levels = levels;
	int i, j;

// create decomposition tree
	values = new double*[2 * levels];
	j = input_length;
	for (i = 0; i < levels; i++)
	{
		j /= 2;
		if (j == 0)
		{
			levels = i;
			continue;
		}
		values[2 * i] = new double[j + 5];
		values[2 * i + 1] = new double[j + 5];
	}
}

Tree::~Tree()
{
	int i;

	for (i = 2 * levels - 1; i >= 0; i--)
		delete values[i];

	delete values;
}

WaveletCoeffs::WaveletCoeffs(double alpha, double beta)
{
	int i;
	double tcosa = cos(alpha);
	double tcosb = cos(beta);
	double tsina = sin(alpha);
	double tsinb = sin(beta);

// calculate first two wavelet coefficients  a = a(-2) and b = a(-1)
	values[0] = ((1.0 + tcosa + tsina) * (1.0 - tcosb - tsinb)
					+ 2.0 * tsinb * tcosa) / 4.0;
	values[1] = ((1.0 - tcosa + tsina) * (1.0 + tcosb - tsinb)
					- 2.0 * tsinb * tcosa) / 4.0;

	tcosa = cos(alpha - beta);
	tsina = sin(alpha - beta);

// calculate last four wavelet coefficients  c = a(0), d = a(1), 
// e = a(2), and f = a(3)
	values[2]  = (1.0 + tcosa + tsina) / 2.0;
	values[3]  = (1.0 + tcosa - tsina) / 2.0;
	values[4]  = 1 - values[0] - values[2];
	values[5]  = 1 - values[1] - values[3];

// zero out very small coefficient values caused by truncation error
	for (i = 0; i < 6; i++)
	{
		if (fabs(values[i]) < 1.0e-15) values[i] = 0.0;
	}
}

WaveletCoeffs::~WaveletCoeffs()
{
}


WaveletFilters::WaveletFilters(WaveletCoeffs *wave_coeffs, wavetype transform)
{
	int i, j, k;

// find the first non-zero wavelet coefficient
	i = 0;
	while(wave_coeffs->values[i] == 0.0) i++;

// find the last non-zero wavelet coefficient
	j = 5;
	while(wave_coeffs->values[j] == 0.0) j--;

// Form the decomposition filters h~ and g~ or the reconstruction
// filters h and g.  The division by 2 in the construction
// of the decomposition filters is for normalization.
	length = j - i + 1;
	for(k = 0; k < length; k++)
	{
		if (transform == DECOMP)
		{
			h[k] = wave_coeffs->values[j--] / 2.0;
			g[k] = (double) (((i++ & 0x01) * 2) - 1) * wave_coeffs->values[i] / 2.0;
		}
		else
		{
			h[k] = wave_coeffs->values[i++];
			g[k] = (double) (((j-- & 0x01) * 2) - 1) * wave_coeffs->values[j];
		}
	}

// clear out the additional array locations, if any
	while (k < 6)
	{
		h[k] = 0.0;
		g[k++] = 0.0;
	}
}

WaveletFilters::~WaveletFilters()
{
}




Denoise::Denoise(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
// triggers the buffer deletion
	thread = 0;

	reset_parameters();
}

Denoise::~Denoise()
{
	delete_buffers();
}

Denoise::delete_buffers()
{
	if(input_buffer)
	{
		delete dsp_in;
		delete dsp_iteration;
		delete dsp_out;
		delete input_buffer;
		delete output_buffer;
		delete ex_coeff_d;
		delete ex_coeff_r;
		delete ex_coeff_rn;
		delete wave_coeff_d;
		delete wave_coeff_r;
		delete decomp_filter;
		delete recon_filter;
	}
	input_buffer = 0;
}


Denoise::reset_parameters()
{
// default parameters
	noise_level = 10.0;
	alpha = 1.359803732;
	beta = -0.782106385;

	lambda1[0] = 1.678; lambda1[1] = 1.893; lambda1[2] = 2.116;
	lambda1[3] = 2.331; lambda1[4] = 2.538; lambda1[5] = 2.737;
	lambda1[6] = 2.930; lambda1[7] = 3.116; lambda1[8] = 3.296;

	lambda2[0] = 8.004; lambda2[1] = 7.980; lambda2[2] = 7.549;
	lambda2[3] = 7.259; lambda2[4] = 7.069; lambda2[5] = 6.939;
	lambda2[6] = 6.848; lambda2[7] = 6.799; lambda2[8] = 6.760;

	output_level = 1;
	window_size = 4096;
// cache required is window_size * 8 * 2 * 6
	levels = 1;
	iterations = 1;
	redo_buffers = 1;       // set to redo buffers before the first render
}


Denoise::init_buffers()
{
	fragment_in_position = in_buffer_size + WINDOWBORDER;
	window_in_position = in_buffer_size + WINDOWBORDER;
	fragment_out_position = window_in_position - window_size - WINDOWBORDER;
	input_buffer = 0;
	input_size = 0;
}

Denoise::redo_buffers_procedure()
{
// get the window size
// window size must be a power of 2
	int new_size = 64;
	while(new_size < window_size && new_size < 16384) new_size *= 2;
	window_size = new_size;

// get the dsp buffer size to be allocated
// only change it if it's bigger
	new_size = window_size + in_buffer_size * 2;
	if(new_size <= input_size) new_size = input_size;
	float *new_input_buffer = new float[new_size];
	float *new_output_buffer = new float[new_size];

// copy data from the old input_buffer
	if(input_buffer)
	{
		int i;
		for(i = 0; i < input_size; i++)
		{
			new_input_buffer[i] = input_buffer[i];
			new_output_buffer[i] = output_buffer[i];
		}
		while(i < new_size)
		{
			new_input_buffer[i] = 0;
			new_output_buffer[i] = 0;
			i++;
		}
	}
	input_size = new_size;
// Changing the window size greater than the fragment size will
// cause glitches since the fragment delay isn't adjusted here.
//	fragment_out_position = fragment_in_position - window_size - in_buffer_size;

	delete_buffers();

// fix these
	if(levels > 8) levels = 8;
	if(iterations > 32) iterations = 32;

// get the actual window buffer size needed
	long size_factor = (int)(pow(2, levels));
	dsp_in = new double[window_size * size_factor];
	dsp_iteration = new double[window_size * 2];
	dsp_out = new double[window_size * 2];
	input_buffer = new_input_buffer;
	output_buffer = new_output_buffer;

// allocate trees
	ex_coeff_d = new Tree(window_size, levels);
	ex_coeff_r = new Tree(window_size, levels);
	ex_coeff_rn = new Tree(window_size, levels);

	wave_coeff_d = new WaveletCoeffs(alpha, beta);
	wave_coeff_r = new WaveletCoeffs(alpha, beta);
	decomp_filter = new WaveletFilters(wave_coeff_d, DECOMP);
	recon_filter = new WaveletFilters(wave_coeff_r, RECON);
	in_scale = 65535 / sqrt(window_size) / iterations;
	out_scale = output_level / 65535 * sqrt(window_size);
	redo_buffers = 0;
}

Denoise::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sdenoise.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	noise_level = defaults->get("NOISELEVEL", (float)10);
	output_level = defaults->get("OUTPUTLEVEL", (float)1);
	window_size = defaults->get("WINDOWSIZE", 4096);
	levels = defaults->get("LEVELS", 1);
	iterations = defaults->get("ITERATIONS", 1);
}

Denoise::save_defaults()
{
	defaults->update("NOISELEVEL", noise_level);
	defaults->update("OUTPUTLEVEL", output_level);
	defaults->update("WINDOWSIZE", window_size);
	defaults->update("LEVELS", levels);
	defaults->update("ITERATIONS", iterations);
	defaults->save();
}

char* Denoise::plugin_title() { return "Denoise2"; }
Denoise::plugin_is_realtime() { return 1; }
Denoise::plugin_is_multi_channel() { return 0; }

Denoise::start_realtime()
{
	init_buffers();
	redo_buffers_procedure();
}

Denoise::stop_realtime()
{
	delete_buffers();
}

Denoise::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	if(redo_buffers) redo_buffers_procedure();

	long new_fragment_in_position = fragment_in_position + size;
	if(new_fragment_in_position > input_size) new_fragment_in_position -= input_size;
	float *input_buffer_end = input_buffer + input_size;
	float *output_buffer_end = output_buffer + input_size;

// copy the new fragment into the input_buffer
	{
		float *output = &input_buffer[fragment_in_position];
		float *output_end = output + size;
		if(output_end >= input_buffer_end) output_end -= input_size;

		while(output != output_end)
		{
			*output++ = *input_ptr++;
			if(output >= input_buffer_end) output = input_buffer;
		}
	}

	fragment_in_position = new_fragment_in_position;

	if(fragment_in_position >= input_size) fragment_in_position -= input_size;

// have enough to process a window
	while((fragment_in_position < window_in_position && input_size - window_in_position + fragment_in_position >= window_size) ||
		(fragment_in_position - window_in_position >= window_size))
	{
// copy input_buffer to the dsp_in
// and clear the dsp_out buffer
		{
			float *input = &input_buffer[window_in_position];
			float *input_end = input + window_size;
			if(input_end > input_buffer_end) input_end -= input_size;
			double *output = dsp_in;
			double *dsp_out_ptr = dsp_out;

			while(input != input_end)
			{
				if(input >= input_buffer_end) input = input_buffer;
				*output = *input++;
				*output++ *= in_scale;
				*dsp_out_ptr++ = 0;
			}
		}
		process_window();


// copy from dsp_out into the output_buffer with crossfading
		{
// crossfade first WINDOWBORDER with last WINDOWBORDER of previous window
			float *output = &output_buffer[window_in_position];
			float *output_end = output + WINDOWBORDER;
			double *input = dsp_out;
			double input_slope = (double)1 / WINDOWBORDER;
			float output_slope = (double)1 / WINDOWBORDER;
			double input_scale = 0;
			float output_scale = 1;
			if(output_end > output_buffer_end) output_end -= input_size; 

			while(output != output_end)
			{
				if(output >= output_buffer_end) output = output_buffer;
				*input *= out_scale;
				*input *= input_scale;
				*output *= output_scale;
				*output++ += *input++;
				input_scale += input_slope;
				output_scale -= output_slope;
			}

// overwrite rest of output with input
			output_end = output + window_size - WINDOWBORDER;
			if(output_end > output_buffer_end) output_end -= input_size; 

			while(output != output_end)
			{
				if(output >= output_buffer_end) output = output_buffer;
				*input *= out_scale;
				*output++ = *input++;
			}
		}

// advance the window position
		window_in_position += window_size - WINDOWBORDER;
		if(window_in_position >= input_size) window_in_position -= input_size;
	}


// write the fragment from the oldest processed window to output
	float *output_end = output_ptr + size;

	if(fragment_out_position < 0)
	{
		float *new_output_end = output_end;
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
}

Denoise::process_window()
{
	int i, j;
	for(j = 0; j < iterations; j++)
	{
		wavelet_decomposition(dsp_in, window_size, ex_coeff_d->values);
		tree_copy(ex_coeff_r->values, ex_coeff_d->values, window_size, levels);

// qualify coeffs
 		threshold(window_size, noise_level, levels);

		wavelet_reconstruction(ex_coeff_r->values, window_size, dsp_iteration);
		wavelet_reconstruction(ex_coeff_rn->values, window_size, dsp_in);

		for(i = 0; i < window_size; i++)
			dsp_out[i] += dsp_iteration[i];
	}
}

Denoise::wavelet_decomposition(double *in_data, long in_length, double **out_data)
{
	for(int i = 0; i < levels; i++)
	{
		in_length = decompose_branches(in_data, in_length, decomp_filter, 
			out_data[2 * i], out_data[(2 * i) + 1]);

		in_data = out_data[2 * i];
	}
}

long Denoise::decompose_branches(double *in_data, long length, WaveletFilters *decomp_filter, double *out_low, double *out_high)
{
// Take input data and filters and form two branches of half the
// original length. Length of branches is returned.
	convolve_dec_2(in_data, length, decomp_filter->h, decomp_filter->length, out_low);
	convolve_dec_2(in_data, length, decomp_filter->g, decomp_filter->length, out_high);
	return (length / 2);
}

Denoise::convolve_dec_2(double *input_sequence, long length,
					double *filter, int filtlen, double *output_sequence)
{
// convolve the input sequence with the filter and decimate by two
	int i, shortlen, offset;
	long lengthp4 = length + 4;
	long lengthm4 = length - 4;
	long lengthp5 = length + 5;
	long lengthp8 = length + 8;

	for(i = 0; (i <= lengthp8) && ((i - filtlen) <= lengthp8); i += 2)
	{
		if(i < filtlen)
			*output_sequence++ = dot_product(input_sequence + i, filter, i + 1);
		else 
		if (i > lengthp5)
		{
			offset = i - lengthm4;
			shortlen = filtlen - offset;
			*output_sequence++ = dot_product(input_sequence + lengthp4,
								filter + offset, shortlen);
		}
		else
			*output_sequence++ = dot_product(input_sequence + i, filter, filtlen);
	}
}

double Denoise::dot_product(double *data, double *filter, char filtlen)
{
	static int i;
	static double sum;

	sum = 0.0;
	for(i = 0; i < filtlen; i++) sum += *data-- * *filter++;
	return sum;
}

Denoise::tree_copy(double **output, double **input, int length, int levels)
{
	register int i, j, k, l, m;

	for(i = 0, k = 1; k < levels; i++, k++)
	{
		length /= 2;
		l = 2 * i;
		m = l + 1;

		for(j = 0; j < length + 5; j++)
		{
			output[l][j] = 0.0;
			output[m][j] = input[m][j];
		}
	}

	length /= 2;
	l = 2 * i;
	m = l + 1;

	for(j = 0; j < length + 5; j++)
	{
		output[l][j] = input[l][j];
		output[m][j] = input[m][j];
	}
}

Denoise::threshold(int window_size, double noise_level, int levels)
{
	int i, j, k, length;
	double l1, l2, v, th;

	k = (int)(log(window_size) / log(2)) - 6;
	l1 = lambda1[k] - lambda1[k] * noise_level / 100;
	l2 = lambda2[k] + lambda2[k] * noise_level / 100;

	for(i = 0; i < levels; i++) 
	{
		length = (window_size >> (i + 1)) + 5;

		for(j = 0; j < length; j++) 
		{
			v = ex_coeff_r->values[(2 * i) + 1][j];
			th = SGN(v) * (fabs(v) - l1) / (l2 - l1);
			
			if(fabs(v) < l1)
			{
				ex_coeff_rn->values[(2 * i) + 1][j] = ex_coeff_r->values[(2 * i) + 1][j];
				ex_coeff_r->values[(2 * i) + 1][j] = 0.0;
			}
			else
			if(fabs(v) > l1 && fabs(v) < l2)
			{
				ex_coeff_r->values[(2 * i) + 1][j] = th;
			} 
			else
				ex_coeff_rn->values[(2 * i) + 1][j] = 0.0;
		}
	}
}

Denoise::wavelet_reconstruction(double **in_data, long in_length, double *out_data)
{
	double *output;
	int i;

	in_length = in_length >> levels;
// destination of all but last branch reconstruction is the next
// higher intermediate approximation
	for(i = levels - 1; i > 0; i--)
	{
		output = in_data[2 * (i - 1)];
		in_length = reconstruct_branches(in_data[2 * i], in_data[(2 * i) + 1],
										in_length, recon_filter, output);
	}

// destination of the last branch reconstruction is the output data
	reconstruct_branches(in_data[0], in_data[1], in_length, recon_filter, out_data);
}

long Denoise::reconstruct_branches(double *in_low, double *in_high, long in_length,
				WaveletFilters *recon_filter, double *output)
{
// take input data and filters and form two branches of half the
// original length. length of branches is returned
	convolve_int_2(in_low, in_length, recon_filter->h, 
					recon_filter->length, 0, output);
	convolve_int_2(in_high, in_length, recon_filter->g, 
					recon_filter->length, 1, output);
	return in_length * 2;
}

Denoise::convolve_int_2(double *input_sequence, long length, 
					double *filter, int filtlen, int sum_output, double *output_sequence)
// insert zeros between each element of the input sequence and
// convolve with the filter to interpolate the data
{
	register int i, j;
	int endpoint = length + filtlen - 2;

	if (sum_output)
	{
// summation with previous convolution
// every other dot product interpolates the data
		for(i = (filtlen / 2) - 1, j = (filtlen / 2); i < endpoint; i++, j++)
		{
			*output_sequence++ += dot_product_odd(input_sequence + i, filter, filtlen);
			*output_sequence++ += dot_product_even(input_sequence + j, filter, filtlen);
		}

		*output_sequence++ += dot_product_odd(input_sequence + i, filter, filtlen);
	}
	else
	{
// first convolution of pair
// every other dot product interpolates the data
		for(i = (filtlen / 2) - 1, j = (filtlen / 2); i < endpoint; i++, j++)
		{
			*output_sequence++ = dot_product_odd(input_sequence + i, filter, filtlen);
			*output_sequence++ = dot_product_even(input_sequence + j, filter, filtlen);
		}

		*output_sequence++ = dot_product_odd(input_sequence + i, filter, filtlen);
	}
}


double Denoise::dot_product_even(double *data, double *filter, int filtlen)
{
	static int i;
	static double sum;

	sum = 0.0;
	for(i = 0; i < filtlen; i += 2) sum += *data-- * filter[i];
	return sum;
}


double Denoise::dot_product_odd(double *data, double *filter, int filtlen)
{
	static int i;
	static double sum;

	sum = 0.0;
	for(i = 1; i < filtlen; i += 2) sum += *data-- * filter[i];
	return sum;
}

Denoise::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new DenoiseThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start

	update_gui();            // fill GUI with parameters
}

Denoise::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

Denoise::show_gui()
{
	thread->window->show_window();
}

Denoise::hide_gui()
{
	thread->window->hide_window();
}

Denoise::set_string()
{
	thread->window->set_title(gui_string);
}

Denoise::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("DENOISE");
	output.tag.set_property("NOISELEVEL", noise_level);
	output.tag.set_property("OUTPUTLEVEL", output_level);
	output.tag.set_property("WINDOWSIZE", window_size);
	output.tag.set_property("LEVELS", levels);
	output.tag.set_property("ITERATIONS", iterations);
	output.append_tag();
	output.append_newline();
	output.terminate_string();
// data is now in *text
}

Denoise::read_data(char *text)
{
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));

	int result = 0;
	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("DENOISE"))
			{
				noise_level = input.tag.get_property("NOISELEVEL", noise_level);
				output_level = input.tag.get_property("OUTPUTLEVEL", output_level);
				window_size = input.tag.get_property("WINDOWSIZE", window_size);
				levels = input.tag.get_property("LEVELS", levels);
				iterations = input.tag.get_property("ITERATIONS", iterations);
			}
		}
	}

	redo_buffers = 1;
	update_gui();
}

Denoise::update_gui()
{
	if(thread)
	{
		thread->window->update_gui();
	}
}

Denoise::reset()
{
	reset_parameters();
	update_gui();
}
