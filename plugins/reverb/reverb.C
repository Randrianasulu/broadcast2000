#include <math.h>
#include <time.h>

#include "confirmsave.h"
#include "errorbox.h"
#include "filehtal.h"
#include "reverb.h"
#include "reverbwindow.h"

int main(int argc, char *argv[])
{
	Reverb *plugin;
	
	srand(time(0));
	plugin = new Reverb(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

Reverb::Reverb(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	redo_buffers = 1;       // set to redo buffers before the first render

	dsp_in_length = 0;
	thread = 0;
}

Reverb::~Reverb()
{
// never called
	delete defaults;
	delete ref_channels;
	delete ref_offsets;
	delete ref_levels;
	delete ref_lowpass;
	delete dsp_in;
}

int Reverb::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%sreverb.rc", get_defaultdir());
	
// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	level_init = defaults->get("LEVEL_INIT", 0);
	delay_init = defaults->get("DELAY_INIT", 100);
	ref_level1 = defaults->get("REF_LEVEL1", -6);
	ref_level2 = defaults->get("REF_LEVEL2", INFINITYGAIN);
	ref_total = defaults->get("REF_TOTAL", 12);
	ref_length = defaults->get("REF_LENGTH", 1000);
	lowpass1 = defaults->get("LOWPASS1", 20000);
	lowpass2 = defaults->get("LOWPASS2", 20000);
	
	sprintf(config_directory, "~");
	defaults->get("CONFIG_DIRECTORY", config_directory);
	
//printf("Reverb::load_defaults ref_level2 %f\n", ref_level2);
return 0;
}

int Reverb::save_defaults()
{
	defaults->update("LEVEL_INIT", level_init);
	defaults->update("DELAY_INIT", delay_init);
	defaults->update("REF_LEVEL1", ref_level1);
	defaults->update("REF_LEVEL2", ref_level2);
	defaults->update("REF_TOTAL", ref_total);
	defaults->update("REF_LENGTH", ref_length);
	defaults->update("LOWPASS1", lowpass1.freq);
	defaults->update("LOWPASS2", lowpass2.freq);
	defaults->update("CONFIG_DIRECTORY", config_directory);
	defaults->save();
return 0;
}


char* Reverb::plugin_title() { return "Reverb"; }
int Reverb::plugin_is_realtime() { return 1; }
int Reverb::plugin_is_multi_channel() { return 1; }

int Reverb::start_realtime()
{
	int i;

	dsp_in = new double*[total_in_buffers];
	ref_channels = new long*[total_in_buffers];
	ref_offsets = new long*[total_in_buffers];
	ref_levels = new float*[total_in_buffers];
	ref_lowpass = new long*[total_in_buffers];
	lowpass_in1 = new double*[total_in_buffers];
	lowpass_in2 = new double*[total_in_buffers];

	for(i = 0; i < total_in_buffers; i++)
	{
		dsp_in[i] = new double[1];
		ref_channels[i] = new long[1];
		ref_offsets[i] = new long[1];
		ref_levels[i] = new float[1];
		ref_lowpass[i] = new long[1];
		lowpass_in1[i] = new double[1];
		lowpass_in2[i] = new double[1];
	}
	
	engine = new ReverbEngine*[smp];
	for(i = 0; i < smp; i++)
	{
		engine[i] = new ReverbEngine(this);
		engine[i]->start();
	}
	redo_buffers = 1;
return 0;
}

int Reverb::stop_realtime()
{
	for(int i = 0; i < total_in_buffers; i++)
	{
		delete dsp_in[i];
		delete ref_channels[i];
		delete ref_offsets[i];
		delete ref_levels[i];
		delete ref_lowpass[i];
		delete lowpass_in1[i];
		delete lowpass_in2[i];
	}
	delete dsp_in;
	delete ref_channels;
	delete ref_offsets;
	delete ref_levels;
	delete ref_lowpass;
	delete lowpass_in1;
	delete lowpass_in2;
	
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
return 0;
}

int Reverb::process_realtime(long size, float **input_ptr, float **output_ptr)
{
	long new_dsp_length, i, j;
	main_in = input_ptr;
	main_out = output_ptr;

	new_dsp_length = in_buffer_size + (delay_init + ref_length) * project_sample_rate / 1000;
	if(redo_buffers || new_dsp_length != dsp_in_length)
	{
		double *old_dsp, *new_dsp;

		for(i = 0; i < total_in_buffers; i++)
		{
			old_dsp = dsp_in[i];
			new_dsp = new double[new_dsp_length];
			for(j = 0; j < dsp_in_length && j < new_dsp_length; j++) new_dsp[j] = old_dsp[j];
			for( ; j < new_dsp_length; j++) new_dsp[j] = 0;
			delete old_dsp;
			dsp_in[i] = new_dsp;
		}

		dsp_in_length = new_dsp_length;
		redo_buffers = 1;
	}

	if(redo_buffers)
	{
		for(i = 0; i < total_in_buffers; i++)
		{
			delete ref_channels[i];
			delete ref_offsets[i];
			delete ref_lowpass[i];
			delete ref_levels[i];
			delete lowpass_in1[i];
			delete lowpass_in2[i];
			
			ref_channels[i] = new long[ref_total + 1];
			ref_offsets[i] = new long[ref_total + 1];
			ref_lowpass[i] = new long[ref_total + 1];
			ref_levels[i] = new float[ref_total + 1];
			lowpass_in1[i] = new double[ref_total + 1];
			lowpass_in2[i] = new double[ref_total + 1];

// set channels			
			ref_channels[i][0] = i;         // primary noise
			ref_channels[i][1] = i;         // first reflection
// set offsets
			ref_offsets[i][0] = 0;
			ref_offsets[i][1] = delay_init * project_sample_rate / 1000;
// set levels
			ref_levels[i][0] = db.fromdb(level_init);
			ref_levels[i][1] = db.fromdb(ref_level1);
// set lowpass
			ref_lowpass[i][0] = -1;     // ignore first noise
			ref_lowpass[i][1] = lowpass1.freq;
			lowpass_in1[i][0] = 0;
			lowpass_in2[i][0] = 0;
			lowpass_in1[i][1] = 0;
			lowpass_in2[i][1] = 0;

			long ref_division = ref_length * project_sample_rate / 1000 / (ref_total + 1);
			for(j = 2; j < ref_total + 1; j++)
			{
// set random channels for remaining reflections
				ref_channels[i][j] = rand() % total_in_buffers;

// set random offsets after first reflection
				ref_offsets[i][j] = ref_offsets[i][1];
				ref_offsets[i][j] += ref_division * j - (rand() % ref_division) / 2;

// set changing levels
				ref_levels[i][j] = db.fromdb(ref_level1 + (ref_level2 - ref_level1) / (ref_total - 1) * (j - 2));
				//ref_levels[i][j] /= 100;

// set changing lowpass as linear
				ref_lowpass[i][j] = (long)(lowpass1.freq + (float)(lowpass2.freq - lowpass1.freq) / (ref_total - 1) * (j - 2));
				lowpass_in1[i][j] = 0;
				lowpass_in2[i][j] = 0;
			}
		}
		
		redo_buffers = 0;
	}

	for(i = 0; i < total_in_buffers; )
	{
		for(j = 0; j < smp && (i + j) < total_in_buffers; j++)
		{
			engine[j]->process_overlays(i + j, size);
		}
		
		for(j = 0; j < smp && i < total_in_buffers; j++, i++)
		{
			engine[j]->wait_process_overlays();
		}
	}

	for(i = 0; i < total_in_buffers; i++)
	{
		float *current_out = main_out[i];
		double *current_in = dsp_in[i];

		for(j = 0; j < size; j++) current_out[j] = current_in[j];

		long k;
		for(k = 0; j < dsp_in_length; j++, k++) current_in[k] = current_in[j];
		
		for(; k < dsp_in_length; k++) current_in[k] = 0;
	}
return 0;
}



int Reverb::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new ReverbThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();
return 0;
}

int Reverb::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
	save_defaults();
return 0;
}

int Reverb::show_gui()
{
	thread->window->show_window();
return 0;
}

int Reverb::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int Reverb::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int Reverb::save_data(char *text)
{
	FileHTAL output;

// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);
	
	output.tag.set_title("REVERB");
	output.tag.set_property("LEVELINIT", level_init);
	output.tag.set_property("DELAY_INIT", delay_init);
	output.tag.set_property("REF_LEVEL1", ref_level1);
	output.tag.set_property("REF_LEVEL2", ref_level2);
	output.tag.set_property("REF_TOTAL", ref_total);
	output.tag.set_property("REF_LENGTH", ref_length);
	output.tag.set_property("LOWPASS1", lowpass1.freq);
	output.tag.set_property("LOWPASS2", lowpass2.freq);
//printf("Reverb::save_data ref_level2 %f\n", ref_level2);
	output.append_tag();
	output.append_newline();
	
	
	
	output.terminate_string();
// data is now in *text
return 0;
}

int Reverb::read_data(char *text)
{
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));
	int result = 0;

	result = input.read_tag();

	if(!result)
	{
		if(input.tag.title_is("REVERB"))
		{
			level_init = input.tag.get_property("LEVELINIT", level_init);
			delay_init = input.tag.get_property("DELAY_INIT", delay_init);
			ref_level1 = input.tag.get_property("REF_LEVEL1", ref_level1);
			ref_level2 = input.tag.get_property("REF_LEVEL2", ref_level2);
			ref_total = input.tag.get_property("REF_TOTAL", ref_total);
			ref_length = input.tag.get_property("REF_LENGTH", ref_length);
			lowpass1 = input.tag.get_property("LOWPASS1", lowpass1.freq);
			lowpass2 = input.tag.get_property("LOWPASS2", lowpass2.freq);
//printf("Reverb::read_data ref_level2 %f\n", ref_level2);
			redo_buffers = 1;
			update_gui();
		}
	}
return 0;
}

int Reverb::update_gui()
{
	if(thread)
	{
		thread->window->level_init->update(level_init);
		thread->window->delay_init->update(delay_init);
		thread->window->ref_level1->update(ref_level1);
		//thread->window->ref_level2->update(ref_level2);
		thread->window->ref_total->update(ref_total);
		thread->window->ref_length->update(ref_length);
		thread->window->lowpass1->update(lowpass1);
		thread->window->lowpass2->update(lowpass2);
	}
return 0;
}




int Reverb::load_from_file(char *path)
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

int Reverb::save_to_file(char *path)
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

ReverbEngine::ReverbEngine(Reverb *plugin)
 : Thread()
{
	this->plugin = plugin;
	completed = 0;
	input_lock.lock();
	output_lock.lock();
}

ReverbEngine::~ReverbEngine()
{
	completed = 1;
	input_lock.unlock();
	join();
}

int ReverbEngine::process_overlays(int output_buffer, long size)
{
	this->output_buffer = output_buffer;
	this->size = size;
	input_lock.unlock();
return 0;
}

int ReverbEngine::wait_process_overlays()
{
	output_lock.lock();
return 0;
}
	
int ReverbEngine::process_overlay(float *in, double *out, double &out1, double &out2, double level, long lowpass, long samplerate, long size)
{
	if(lowpass == -1 || lowpass >= plugin->project_sample_rate / 2 - 2500)
	{
// no lowpass filter
		for(register int i = 0; i < size; i++) out[i] += in[i] * level;
	}
	else
	{
		register double coef = 0.25 * 2.0 * M_PI * (double)lowpass / (double)plugin->project_sample_rate;
		register double a = coef * 0.25;
		register double b = coef * 0.50;

		for(register int i = 0; i < size; i++)
		{
			out2 += a * (3 * out1 + in[i] - out2);
			out2 += b * (out1 + in[i] - out2);
			out2 += a * (out1 + 3 * in[i] - out2);
			out2 += coef * (in[i] - out2);
			out1 = in[i];
			out[i] += out2 * level;
		}
	}
return 0;
}

void ReverbEngine::run()
{
	int j, i;
	while(1)
	{
		input_lock.lock();
		if(completed) return;

// Process reverb
		for(i = 0; i < plugin->total_in_buffers; i++)
		{
			for(j = 0; j < plugin->ref_total + 1; j++)
			{
				if(plugin->ref_channels[i][j] == output_buffer)
					process_overlay(plugin->main_in[i], 
								&(plugin->dsp_in[plugin->ref_channels[i][j]][plugin->ref_offsets[i][j]]), 
								plugin->lowpass_in1[i][j], 
								plugin->lowpass_in2[i][j], 
								plugin->ref_levels[i][j], 
								plugin->ref_lowpass[i][j], 
								plugin->project_sample_rate, 
								size);
			}
		}

		output_lock.unlock();
	}
}
