#include <math.h>

#include "confirmsave.h"
#include "errorbox.h"
#include "filehtal.h"
#include "compress.h"
#include "compresswindow.h"

int main(int argc, char *argv[])
{
	Compress *plugin;

	plugin = new Compress(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

Compress::Compress(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	redo_buffers = 1;       // set to redo buffers before the first render

	dsp_in_length = 0;
	thread = 0;
}

Compress::~Compress()
{
// never called
	delete defaults;
	delete dsp_in;
}

int Compress::load_defaults()
{
	char directory[1024], string[1024];

// set the default directory
	sprintf(directory, "%scompress.rc", BCASTDIR);
	
// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	sprintf(config_directory, BCASTDIR);
	defaults->get("CONFIG_DIRECTORY", config_directory);

	readahead = defaults->get("READAHEAD", 0);
	attack = defaults->get("ATTACK", 1000);
	channel = defaults->get("CHANNEL", 1);
	total_points = defaults->get("TOTAL_POINTS", 0);
	for(int i = 0; i < total_points; i++)
	{
		sprintf(string, "INPUT%d", i);
		input_level[i] = defaults->get(string, 0);
		sprintf(string, "OUTPUT%d", i);
		output_level[i] = defaults->get(string, input_level[i]);
	}
}

int Compress::save_defaults()
{
	char string[1024];

	defaults->update("CONFIG_DIRECTORY", config_directory);
	defaults->update("READAHEAD", readahead);
	defaults->update("ATTACK", attack);
	defaults->update("CHANNEL", channel);
	defaults->update("TOTAL_POINTS", total_points);
	for(int i = 0; i < total_points; i++)
	{
		sprintf(string, "INPUT%d", i);
		defaults->update(string, input_level[i]);
		sprintf(string, "OUTPUT%d", i);
		defaults->update(string, output_level[i]);
	}
	defaults->save();
}


char* Compress::plugin_title() { return "Compress"; }
int Compress::plugin_is_realtime() { return 1; }
int Compress::plugin_is_multi_channel() { return 1; }

int Compress::start_realtime()
{
	int i;

// get extra buffers
	coefficients = new float[in_buffer_size];
	dsp_in = new float*[total_in_buffers];

	for(i = 0; i < total_in_buffers; i++)
	{
		dsp_in[i] = new float[1];
		dsp_in[i][0] = 0;
	}
	
	redo_buffers = 1;
	peak_sample1 = 0;
	peak_sample2 = 0;
	peak_value1 = 0;
	peak_value2 = 0;
	current_slope = 0;
}

int Compress::stop_realtime()
{
	delete coefficients;
	for(int i = 0; i < total_in_buffers; i++)
	{
		delete dsp_in[i];
	}
	delete dsp_in;
}

int Compress::process_realtime(long size, float **input_ptr, float **output_ptr)
{
	static int i, real_channel;

	if(redo_buffers) redo_buffers_procedure();

	for(i = 0; i < total_in_buffers; i++) { import_data(i, size, input_ptr[i]); }

	real_channel = channel - 1;
	if(real_channel < 0) real_channel = 0;
	if(real_channel >= total_in_buffers) real_channel = total_in_buffers - 1;

	negotiate_coefficients(real_channel, size);

	for(i = 0; i < total_in_buffers; i++) 
	{
		export_data(i, size, output_ptr[i]); 
	}
}

int Compress::redo_buffers_procedure()
{
	long old_dsp_length;
	float *old_dsp, *new_dsp;
	int i, j;

	attack_samples = attack * project_sample_rate / 1000;
	readahead_samples = readahead * project_sample_rate / 1000;
	old_dsp_length = dsp_length;
	dsp_length = readahead_samples + in_buffer_size + 16;
	last_point = total_points - 1;

	for(i = 0; i < total_in_buffers; i++)
	{
		old_dsp = dsp_in[i];
		new_dsp = new float[dsp_length];

		if(old_dsp_length > dsp_length) old_dsp_length = dsp_length;
		for(j = 0; j < old_dsp_length; j++) new_dsp[j] = old_dsp[j];
		for( ; j < dsp_length; j++) new_dsp[j] = 0;

		delete dsp_in[i];
		dsp_in[i] = new_dsp;
	}

	for(i = 0; i < total_points; i++)
	{
// protect against infinite slopes during envelope adjustments
		if(i > 0 && input_level[i] - input_level[i-1] >= 0.01)
		{
			input_power[i] = db.fromdb(input_level[i]);
			output_power[i] = db.fromdb(output_level[i]);
//printf("%f %f %f %f %f\n", input_level[i], output_level[i], input_power[i], output_power[i], output_power[i] /  input_power[i]);
		}
		else
		{
			input_power[i] = db.fromdb(input_level[i] + 0.01);
			output_power[i] = db.fromdb(output_level[i]);
		}
	}

	if(total_points > 1)
	{
		current_output1 = output_power[0];
		current_power1 = input_power[0];
		current_output2 = output_power[1];
		current_power2 = input_power[1];
		current_output_slope = get_slope(current_power1, current_output1, current_power2, current_output2);
	}

	if(total_points)
	{
		if(input_power[0]) 
			first_slope = output_power[0] / input_power[0];
		else
			first_slope = 1;
		
		if(input_power[last_point] < 1)
			last_slope = get_slope(input_power[last_point], output_power[last_point], 1, 1);
		else
			last_slope = 1;
	}
	
	redo_buffers = 0;
}

int Compress::import_data(int channel, long size, float *input_ptr)
{
	float *output = dsp_in[channel];
	
	for(register int i = 0, j = readahead_samples; i < size; i++, j++)
	{ output[j] = input_ptr[i]; }
}

int Compress::export_data(int channel, long size, float *output_ptr)
{
	float *input = dsp_in[channel];
	register int i, j;
	long endpoint = readahead_samples + size;

	for(i = 0; i < size; i++)
	{ 
		output_ptr[i] = input[i] * coefficients[i]; 
	}

// must be a seperate step since readahead can be zero when j = size above
	for(i = 0, j = size; j < endpoint; i++, j++) { input[i] = input[j]; }
}

int Compress::negotiate_coefficients(int channel, long size)
{
	float *input = &dsp_in[channel][readahead_samples];
	long sample;
	float test_slope;
	float current_value;

	for(sample = 0; sample < size; sample++)
	{
// End of last slope range.  Get new slope.
		if(peak_sample2 <= sample)
		{
			peak_sample1 = peak_sample2;
			peak_value1 = peak_value2;
			peak_sample2 = sample + attack_samples;
			peak_value2 = input[sample];
			current_slope = (peak_value2 - peak_value1) / attack_samples;
			current_value = get_current_value(sample);
		}
		else
		{
			current_value = get_current_value(sample);
			if(input[sample] > peak_value2)
			{
				current_slope = (input[sample] - current_value) / attack_samples;
				peak_sample1 = sample;
				peak_value1 = current_value;
				peak_sample2 = sample + attack_samples;
				peak_value2 = input[sample];
			}
		}

		coefficients[sample] = get_coefficient(current_value);
	}
	peak_sample1 -= size;
	peak_sample2 -= size;
}

float Compress::get_current_value(long sample)
{
	return peak_value1 + current_slope * (sample - peak_sample1);
}

float Compress::get_coefficient(float input)
{
	static long current_point;
	static float result;

	if(input == 0)
	{
// floating point exception
		return 0;
	}
	else
	if(total_points == 0)
	{
		result = input;
	}
	else
	if(input <= input_power[0])
	{
		result = input * first_slope;
	}
	else
	if(input > input_power[last_point])
	{
		result = output_power[last_point] + (input - input_power[last_point]) * last_slope;
	}
	else
	{
		if(input < current_power1 || input > current_power2)
		{
			for(current_point = 1; input > input_power[current_point]; current_point++)
			{ ; }
	
			current_output1 = output_power[current_point - 1];
			current_power1 = input_power[current_point - 1];
			current_output2 = output_power[current_point];
			current_power2 = input_power[current_point];
			current_output_slope = get_slope(current_power1, current_output1, current_power2, current_output2);
		}
		
		result = current_output1 + (input - current_power1) * current_output_slope;
	}
	
	return result / input;
}

float Compress::get_slope(long sample1, float value1, long sample2, float value2)
{
	return (value2 - value1) / (sample2 - sample1);
}

float Compress::get_slope(float power1, float scale1, float power2, float scale2)
{
	return (scale2 - scale1) / (power2 - power1);
}


int Compress::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new CompressThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();
}

int Compress::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
	save_defaults();
}

int Compress::show_gui()
{
	thread->window->show_window();
}

int Compress::hide_gui()
{
	thread->window->hide_window();
}

int Compress::set_string()
{
	thread->window->set_title(gui_string);
}

int Compress::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("COMPRESS");
	output.tag.set_property("READAHEAD", readahead);
	output.tag.set_property("ATTACK", attack);
	output.tag.set_property("CHANNEL", channel);
	output.tag.set_property("TOTAL_POINTS", total_points);
	output.append_tag();
	output.append_newline();

	for(int i = 0; i < total_points; i++)
	{
		output.tag.set_title("POINT");
		output.tag.set_property("IN", input_level[i]);
		output.tag.set_property("OUT", output_level[i]);
		output.append_tag();
		output.append_newline();
	}

	output.terminate_string();
// data is now in *text
}

int Compress::read_data(char *text)
{
//printf("Compress::read_data 1\n");
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));

	int result = 0;
	int current_point = 0;
	
	total_points = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("COMPRESS"))
			{
				readahead = input.tag.get_property("READAHEAD", readahead);
				attack = input.tag.get_property("ATTACK", attack);
				channel = input.tag.get_property("CHANNEL", channel);
				total_points = input.tag.get_property("TOTAL_POINTS", total_points);
			}
			else
			if(input.tag.title_is("POINT"))
			{
				input_level[current_point] = input.tag.get_property("IN", 0);
				output_level[current_point] = input.tag.get_property("OUT", 0);
				current_point++;
			}
		}
	}
	redo_buffers = 1;
	update_gui();
//printf("Compress::read_data 2\n");
}

int Compress::update_gui()
{
	if(thread)
	{
		thread->window->draw_envelope();
		thread->window->readahead->update(readahead);
		thread->window->attack->update(attack);
		thread->window->channel->update(channel);
	}
}




int Compress::load_from_file(char *path)
{
	FILE *in;
	int result = 0;
	int length;
	char string[1024];
	
	if(in = fopen(path, "rb"))
	{
		fseek(in, 0, SEEK_END);
		length = ftell(in);
		fseek(in, 0, SEEK_SET);
		fread(string, length, 1, in);
		fclose(in);
		read_data(string);
	}
	else
	{
		perror("fopen:");
// failed
		ErrorBox errorbox("");
		char string[1024];
		sprintf(string, "Couldn't open %s.", path);
		errorbox.create_objects(string);
		errorbox.run_window();
		result = 1;
	}
	
	return result;
}

int Compress::save_to_file(char *path)
{
	FILE *out;
	int result = 0;
	char string[1024];
	
	{
		ConfirmSave confirm;
		result = confirm.test_file("", path);
	}
	
	if(!result)
	{
		if(out = fopen(path, "wb"))
		{
			save_data(string);
			fwrite(string, strlen(string), 1, out);
			fclose(out);
		}
		else
		{
			result = 1;
// failed
			ErrorBox errorbox("");
			char string[1024];
			sprintf(string, "Couldn't save %s.", path);
			errorbox.create_objects(string);
			errorbox.run_window();
			result = 1;
		}
	}
	
	return result;
}
