#include "errorbox.h"
#include "samplerate.h"
#include "sratewindow.h"

#include <math.h>

int main(int argc, char *argv[])
{
	SampleRateMain *plugin;
	
	plugin = new SampleRateMain(argc, argv);
	plugin->plugin_run();
}

SampleRateMain::SampleRateMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	output_rate = 44100;
	input_sample_rate = 44100;
	defaults = 0;
}

SampleRateMain::~SampleRateMain()
{
	if(defaults) delete defaults;
}

int SampleRateMain::run_client()
{
	plugin_exit();
return 0;
}

const char* SampleRateMain::plugin_title() { return "Resample"; }

int SampleRateMain::plugin_is_realtime() { return 0; }

int SampleRateMain::plugin_is_multi_channel() { return 0; }

int SampleRateMain::get_plugin_samplerate() { return output_rate; }

long SampleRateMain::get_in_buffers(long recommended_size)
{
	//return 4 * 0x400;
	return 64000;
}

long SampleRateMain::get_out_buffers(long recommended_size)
{
	//return 2 * 0x400;
	return 64000;
}

int SampleRateMain::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%ssamplerate.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	output_rate = defaults->get("OUTPUTRATE", 44100);
return 0;
}

int SampleRateMain::save_defaults()
{
//printf("save_defaults\n");
	defaults->update("OUTPUTRATE", output_rate);
	defaults->save();
return 0;
}

int SampleRateMain::get_parameters()
{
	int sample_rate_conflicts;
	int result;

	input_sample_rate = get_project_samplerate();

	do{
		sample_rate_conflicts = 0;
		{
			SRateWindow window;
			window.create_objects(&output_rate);
			result = window.run_window();
		}

		if(!result)
		{
			if(input_sample_rate == output_rate)
			{
				sample_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new sample rate is the", "same as the old sample rate.");
				window.run_window();
			}
			else
			if(output_rate > 200000 || output_rate < 4000)
			{
				sample_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new sample rate is out of range.");
				window.run_window();
			}
		}
	}while(sample_rate_conflicts);
	
	send_completed();
	return result;
}


double SampleRateMain::sinc(double x)
{
	return (fabs(x) < 1E-50 ? 1.0 : sin(fmod(x, 2 * M_PI)) / x);
}


double SampleRateMain::interpolate_function(double t, double filter_cutoff_freq, double window_key_freq)
{
	double result;
	
	result = 2;
	result *= filter_cutoff_freq;
	result *= sinc(M_PI * 2 * filter_cutoff_freq * t);
	result *= exp(-M_PI * sqr(2 * window_key_freq * t));
	
	return result;
}


double SampleRateMain::get_coefficient(int i, int q, 
										 int filter_length, 
										 double filter_cutoff_freq, 
										 double window_key_freq, 
										 double input_freq, 
										 double upsampling_factor, 
										 int downsampling_factor, 
										 double gain)
{
	double result;

	result = gain;
	result *=
	interpolate_function(
							(fmod(q * downsampling_factor / upsampling_factor, 1.0) + (filter_length - 1) / 2.0 - i) / input_freq,
							filter_cutoff_freq, 
							window_key_freq
							        );
							          
	result /= input_freq;

	return result;
}


int SampleRateMain::build_coeffient_array()
{
	coefficient_array = new double[filter_length * upsampling_factor];

	int i, q;
	for(i = 0; i < filter_length; i++)
	{
		for(q = 0; q < upsampling_factor; q++)
		{
			coefficient_array[q * filter_length + i] = 
									 get_coefficient(i, q, 
												 filter_length, 
												 filter_cutoff_freq, 
												 window_key_freq, 
												 input_freq, 
												 upsampling_factor, 
												 downsampling_factor, 
												 gain);
		}
	}
	return 0;
}

int SampleRateMain::largest_factor(int x, int y)
{
	int denominator, result;
	int least = x < y ? x : y;
	
	for(denominator = least, result = 0; denominator > 0 && !result; denominator--)
	{
		if(x % denominator == 0 && y % denominator == 0) result = 1; 
	}
	denominator++;
	return denominator;
}

int SampleRateMain::transfer_buffer(double *in, double *out, int len)
{
	double *end;
	
	if(len < 1) return 1;
	
	end = out + len;
	while(out != end) *out++ = *in++;
return 0;
}

int SampleRateMain::apply_filter(double *dsp_in, double *coefficient_array, int filter_length, double *dsp_out)
{
	double result = 0, *coefficient_end;
	
	coefficient_end = coefficient_array + filter_length;
	
	while(coefficient_array != coefficient_end)
	{
		result += *dsp_in++ * *coefficient_array++;
	}

	*dsp_out = result;
return 0;
}

int SampleRateMain::process_buffer(double *dsp_in, 
							 int input_size, 
							 double *dsp_out,
							 int output_size,
							 double *coefficient_array,
							 int filter_length,
							 int upsampling_factor,
							 int downsampling_factor)
{
	while(-1)
	{
		in_offset = (cycle_counter * downsampling_factor) / upsampling_factor;

// need more input data
		if((in_start + in_offset + filter_length) > input_size)
		{
			in_start -= input_size - filter_length + 1;
			return out_start;
		}
		
		apply_filter(dsp_in + in_offset + in_start,
			coefficient_array + cycle_counter * filter_length,
			filter_length,
			dsp_out + out_start++);

		cycle_counter++;
		
		if(!(cycle_counter %= upsampling_factor)) in_start += downsampling_factor;
		if(!(out_start %= output_size)) return output_size;
	}
return 0;
}

int SampleRateMain::write_dsp(double *dsp_out, long out_buffer_size)
{
	for(int i = 0; i < out_buffer_size; i++)
	{
		buffer_out[i] = dsp_out[i];
	}
	return write_samples(out_buffer_size);
}







int SampleRateMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;
	
	if(interactive)
	{
		progress = new BC_ProgressBox("", "Samplerate", end - start);
		progress->start();
	}

// get standard data buffers
	buffer_in = (float*)data_in[0]->get_data();
	buffer_out = (float*)data_out[0]->get_data();

// get high resulution buffers for processing
	double *dsp_in, *dsp_out;
	dsp_in = new double[in_buffer_size];
	dsp_out = new double[out_buffer_size];
	
// get coefficient parameters
	input_freq = 1;
//	gain = 0.8;
	gain = 0.9;

	input_sample_rate = get_project_samplerate();

	int denominator = largest_factor(input_sample_rate, output_rate);
	upsampling_factor = output_rate / denominator;
	downsampling_factor = input_sample_rate / denominator;

	if(output_rate > input_sample_rate)
	{
		window_key_freq = 0.0116;
		filter_cutoff_freq = 0.461;
		filter_length = 162;
	}
	else
	{
		window_key_freq = 0.0116 * upsampling_factor / downsampling_factor;
		filter_cutoff_freq = 0.461 * upsampling_factor / downsampling_factor;
		filter_length = 162 * upsampling_factor / downsampling_factor;
	}




	build_coeffient_array();

	int i, j;
	int result = 0;
	int skirt_length = (filter_length - 1);
	for(i = 0; i < skirt_length; i++) dsp_in[i] = 0;
	long input_advance = in_buffer_size - skirt_length;
	long output_advance;
	in_start = in_offset = cycle_counter = out_start = 0;

	for(long input_position = start; input_position < end && !result; input_position += input_advance)
	{
		if(end - input_position < input_advance) input_advance = end - input_position;
		
// returns 1 for failure
		result = read_samples(input_position, input_advance);
	
		if(!result)
		{
// transfer to high resolution buffer
			for(i = 0, j = skirt_length; i < input_advance; i++, j++)
			{
				dsp_in[j] = buffer_in[i];
			}
			
			do
			{
// process input buffer into output buffer
				output_advance = process_buffer(dsp_in, 
												skirt_length + input_advance, 
												dsp_out,
												out_buffer_size,
												coefficient_array,
												filter_length,
												upsampling_factor,
												downsampling_factor);

// read some more data before writing output
				if(output_advance != out_buffer_size)
				{
					transfer_buffer(dsp_in + input_advance, dsp_in, skirt_length);
					break;
				}

// the output buffer is full so write it
// write input buffer as test
				if(write_dsp(dsp_out, out_buffer_size))
				{
					result = 1;
					break;
				}
			}while(-1);
		}
		if(!result && interactive) result = progress->update(input_position + input_advance - start);
		if(interactive && progress->cancelled()) send_cancelled();
	}
	
	for(i = skirt_length; i < skirt_length * 2; i++) dsp_in[i] = 0;

	if(!result)
	do 
	{
// process last input buffer
		output_advance = process_buffer(dsp_in, 
										skirt_length + skirt_length, 
										dsp_out,
										out_buffer_size,
										coefficient_array,
										filter_length,
										upsampling_factor,
										downsampling_factor);

// write data segment
// write original buffer as test
		if(write_dsp(dsp_out, output_advance))
		{
			result = 1;
			break;
		}
	}while(output_advance == out_buffer_size); 
			
	delete coefficient_array;
	delete dsp_in;
	delete dsp_out;
	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
	

	return result;
}
