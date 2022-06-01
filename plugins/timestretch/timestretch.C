#include "errorbox.h"
#include "timestretch.h"
#include "timewindow.h"

#include <math.h>
#include "timestretch.h"
#include "timewindow.h"

int main(int argc, char *argv[])
{
	TimeStretch *plugin;
	
	plugin = new TimeStretch(argc, argv);
	plugin->plugin_run();
}

ResampleEngine::ResampleEngine(TimeStretch *timestretch)
{
	this->timestretch = timestretch;
	this->input_rate = timestretch->input_rate;
	this->output_rate = (long)((float)input_rate * timestretch->percentage / 100);
}

ResampleEngine::~ResampleEngine()
{
}

double ResampleEngine::sinc(double x)
{
	return (fabs(x) < 1E-50 ? 1.0 : sin(fmod(x, 2 * M_PI)) / x);
}


double ResampleEngine::interpolate_function(double t, double filter_cutoff_freq, double window_key_freq)
{
	double result;
	
	result = 2;
	result *= filter_cutoff_freq;
	result *= sinc(M_PI * 2 * filter_cutoff_freq * t);
	result *= exp(-M_PI * sqr(2 * window_key_freq * t));
	
	return result;
}


double ResampleEngine::get_coefficient(int i, int q, 
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


int ResampleEngine::build_coeffient_array()
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

int ResampleEngine::largest_factor(int x, int y)
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

int ResampleEngine::transfer_buffer(double *in, double *out, int len)
{
	double *end;
	
	if(len < 1) return 1;
	
	end = out + len;
	while(out != end) *out++ = *in++;
return 0;
}

int ResampleEngine::apply_filter(double *dsp_in, double *coefficient_array, int filter_length, double *dsp_out)
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

long ResampleEngine::process_buffer(double *dsp_in, 
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
}

int ResampleEngine::write_dsp(float *buffer_out, double *dsp_out, long buffer_size)
{
	for(int i = 0; i < buffer_size; i++)
	{
		buffer_out[i] = dsp_out[i];
	}
	return write_buffer(buffer_size);
}

int ResampleEngine::do_resample(float *buffer_in, 
					float *buffer_out, 
					long buffer_size, 
					long start, 
					long end)
{
	this->buffer_size = buffer_size;
// thread out progress
	BC_ProgressBox *progress;
	double *dsp_in, *dsp_out;
	
	if(timestretch->interactive)
	{
		progress = new BC_ProgressBox("", "Time Stretch", end - start);
		progress->start();
	}

// get high resulution buffers for processing
	dsp_in = new double[buffer_size];
	dsp_out = new double[buffer_size];

// get coefficient parameters
	input_freq = 1;
	gain = 0.8;

	int denominator = largest_factor(input_rate, output_rate);
	upsampling_factor = output_rate / denominator;
	downsampling_factor = input_rate / denominator;

	if(output_rate > input_rate)
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
	long input_advance;
	long output_advance, input_position;

	in_start = in_offset = cycle_counter = out_start = 0;
	for(i = 0; i < skirt_length; i++) dsp_in[i] = 0;

	for(input_position = start; input_position < end && !result; input_position += input_advance)
	{
		input_advance = buffer_size - skirt_length;
		if(end - input_position < input_advance) input_advance = end - input_position;

// returns 1 for failure
		result = read_buffer(input_position, input_advance);

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
												buffer_size,
												coefficient_array,
												filter_length,
												upsampling_factor,
												downsampling_factor);

// read some more data before writing output
				if(output_advance != buffer_size)
				{
					transfer_buffer(dsp_in + input_advance, dsp_in, skirt_length);
					break;
				}

// the output buffer is full so write it
// write input buffer as test
				if(write_dsp(buffer_out, dsp_out, buffer_size))
				{
					result = 1;
					break;
				}
			}while(-1);
		}
		if(!result && timestretch->interactive) result = progress->update(input_position + input_advance - start);
		if(timestretch->interactive && progress->cancelled()) timestretch->send_cancelled();
	}
	
	for(i = skirt_length; i < skirt_length * 2; i++) dsp_in[i] = 0;

	if(!result)
	do 
	{
// process last input buffer
		output_advance = process_buffer(dsp_in, 
										skirt_length + skirt_length, 
										dsp_out,
										buffer_size,
										coefficient_array,
										filter_length,
										upsampling_factor,
										downsampling_factor);

// write data segment
// write original buffer as test
		if(write_dsp(buffer_out, dsp_out, output_advance))
		{
			result = 1;
			break;
		}
	}while(output_advance == buffer_size); 
			
	delete coefficient_array;
	delete dsp_in;
	delete dsp_out;
	if(timestretch->interactive)
	{
		progress->stop_progress();
		delete progress;
	}
	return result;
}



PitchEngine::PitchEngine(TimeStretch *timestretch)
 : CrossfadeFFT()
{
	this->timestretch = timestretch;
}
PitchEngine::~PitchEngine()
{
}

int PitchEngine::signal_process()
{
    int h = timestretch->window_size / 2;
	double destination, upper, lower;
    int i, dest_i, new_i;
	double freq_scale = (double)timestretch->percentage / 100;

	if(freq_scale < 1)
	{
		for(i = 0; i <= h; i++)
		{
			destination = i * freq_scale;
			dest_i = (int)(destination + 0.5);

			if(dest_i != i)
			{
				freq_real[dest_i] = freq_real[i];
				freq_imag[dest_i] = freq_imag[i];
				freq_real[i] = 0;
				freq_imag[i] = 0;
			}
		}
	}
	else
	if(freq_scale > 1)
	{
		for(i = h; i >= 0; i--)
		{
			destination = i * freq_scale;
			dest_i = (int)(destination + 0.5);

			if(dest_i != i)
			{
				if(dest_i <= h)
				{
					freq_real[dest_i] = freq_real[i];
					freq_imag[dest_i] = freq_imag[i];
				}
				
				freq_real[i] = 0;
				freq_imag[i] = 0;
			}
		}
	}

	symmetry(window_size, freq_real, freq_imag);
return 0;
}





ExpandEngine::ExpandEngine(TimeStretch *timestretch)
 : ResampleEngine(timestretch)
{
	this->timestretch = timestretch;
}

ExpandEngine::~ExpandEngine()
{
}

int ExpandEngine::read_buffer(long position, long size)
{
// buffer_in -> samplerate -> temp_out -> pitch -> buffer_out
	int result = 0;

	result = timestretch->read_samples(position, size);

	return result;
}

int ExpandEngine::write_buffer(long size)
{
// buffer_in -> samplerate -> temp_out -> pitch -> buffer_out
	int result = 0;

// Pitch shift samples
	timestretch->pitch->process_fifo(size, timestretch->temp_out, timestretch->buffer_out);

	if(timestretch->leader_size >= size)
	{
// Don't write leader
		timestretch->leader_size -= size;
	}
	else
	if(timestretch->leader_size > 0)
	{
// Shift samples forward.
		size -= timestretch->leader_size;
		for(int i = 0, j = timestretch->leader_size; i < size; i++, j++)
		{
			timestretch->buffer_out[i] = timestretch->buffer_out[j];
		}
		timestretch->leader_size = 0;
		result = timestretch->write_samples(size);
	}
	else
	{
		result = timestretch->write_samples(size);
	}
	return result;
}







ShrinkEngine::ShrinkEngine(TimeStretch *timestretch)
 : ResampleEngine(timestretch)
{
	this->timestretch = timestretch;
}
ShrinkEngine::~ShrinkEngine()
{
}

int ShrinkEngine::read_buffer(long position, long size)
{
// buffer_in -> pitch -> temp_out -> samplerate -> buffer_out
	int result = 0, temp_size;

// Pitch shift leader
	while(timestretch->leader_size)
	{
		temp_size = timestretch->in_buffer_size;
		if(temp_size > timestretch->leader_size)
			temp_size = timestretch->leader_size;

		for(int i = 0; i < temp_size; i++)
		{
			timestretch->buffer_in[i] = 0;
		}

		timestretch->pitch->process_fifo(size, timestretch->buffer_in, timestretch->temp_out);
		timestretch->leader_size -= temp_size;
	}

// Load data and pitch shift
	result = timestretch->read_samples(position, size);
	if(!result)
		timestretch->pitch->process_fifo(size, timestretch->buffer_in, timestretch->temp_out);

	return result;
}

int ShrinkEngine::write_buffer(long size)
{
// buffer_in -> pitch -> temp_out -> samplerate -> buffer_out
	int result = 0;

	result = timestretch->write_samples(size);

	return result;
}








TimeStretch::TimeStretch(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	percentage = 100;
	window_size = 8192;
	defaults = 0;
}

TimeStretch::~TimeStretch()
{
	if(defaults) delete defaults;
}

char* TimeStretch::plugin_title() { return "Time Stretch"; }

int TimeStretch::plugin_is_realtime() { return 0; }

int TimeStretch::plugin_is_multi_channel() { return 0; }

int TimeStretch::get_plugin_samplerate() { return -1; }

long TimeStretch::get_in_buffers(long recommended_size)
{
	return 65536;
}

long TimeStretch::get_out_buffers(long recommended_size)
{
	return 65536;
}

int TimeStretch::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%ssamplerate.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	percentage = defaults->get("PERCENTAGE", percentage);
	window_size = defaults->get("WINDOWSIZE", window_size);
return 0;
}

int TimeStretch::save_defaults()
{
	defaults->update("PERCENTAGE", percentage);
	defaults->update("WINDOWSIZE", window_size);
	defaults->save();
return 0;
}

int TimeStretch::get_parameters()
{
	int sample_rate_conflicts;
	int result;

	input_rate = get_project_samplerate();

	do{
		sample_rate_conflicts = 0;
		{
			TimeWindow window;
			window.create_objects(this);
			result = window.run_window();
		}

		if(!result)
		{
			if(percentage == 100)
			{
				sample_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new duration is the", "same as the old duration.");
				window.run_window();
			}
			else
			if(percentage > 200 || percentage < 50)
			{
				sample_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new duration is out of range.");
				window.run_window();
			}
		}
	}while(sample_rate_conflicts);

	send_completed();
	return result;
}


int TimeStretch::start_plugin()
{
	ResampleEngine *engine;
	int result = 0;

// get standard data buffers
	buffer_in = (float*)data_in[0]->get_data();
	buffer_out = (float*)data_out[0]->get_data();

	input_rate = get_project_samplerate();
	temp_out = new float[out_buffer_size];
	pitch = new PitchEngine(this);
	pitch->window_size = window_size;
	pitch->init_fft(in_buffer_size);
	pitch->reconfigure();
	leader_size = pitch->get_delay();

	if(percentage > 100)
	{
		engine = new ExpandEngine(this);
		result = engine->do_resample(buffer_in, 
				temp_out, 
				in_buffer_size, 
				start,
				end + leader_size);
	}
	else
	{
		engine = new ShrinkEngine(this);
		result = engine->do_resample(temp_out, 
				buffer_out, 
				in_buffer_size, 
				start,
				end + leader_size);
	}


	delete engine;
	delete pitch;
	delete temp_out;

	return result;
}





