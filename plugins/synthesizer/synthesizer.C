#include <math.h>

#include "confirmsave.h"
#include "errorbox.h"
#include "filehtal.h"
#include "synthesizer.h"
#include "synthmenu.h"
#include "synthwindow.h"

main(int argc, char *argv[])
{
	Synth *plugin;

	plugin = new Synth(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

Synth::Synth(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	redo_buffers = 1;       // set to redo buffers before the first render
	w = 640;
	h = 400;

	reset_parameters();
}

Synth::~Synth()
{
// never called
	delete defaults;
}

int Synth::reset_parameters()
{
	waveform_length = 0;
	dsp_buffer = 0;
	thread = 0;
}

int Synth::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%ssynthesizer.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	sprintf(config_directory, BCASTDIR);
	defaults->get("CONFIG_DIRECTORY", config_directory);
	base_freq = defaults->get("BASEFREQ", 440);
	wavefunction = defaults->get("WAVEFUNCTION", 0);
	w = defaults->get("WIDTH", 380);
	h = defaults->get("HEIGHT", 400);
	total_oscillators = defaults->get("OSCILLATORS", TOTALOSCILLATORS);
	create_oscillators(total_oscillators);
	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->load_defaults(defaults);
	}
}

int Synth::save_defaults()
{
	char string[1024];

	defaults->update("CONFIG_DIRECTORY", config_directory);
	defaults->update("BASEFREQ", base_freq);
	defaults->update("WAVEFUNCTION", wavefunction);
	defaults->update("OSCILLATORS", total_oscillators);
	defaults->update("WIDTH", w);
	defaults->update("HEIGHT", h);
	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->save_defaults(defaults);
	}
	if(thread) thread->window->save_defaults(defaults);
	defaults->save();
}

int Synth::create_oscillators(int total)
{
	total_oscillators = total;

	for(int i = 0; i < total; i++)
	{
		oscillators[i] = new SynthOscillator(this, i);
		oscillators[i]->create_objects(i * OSCILLATORHEIGHT);
	}
}

int Synth::add_oscillator()
{
	if(total_oscillators > 20) return 1;

	int i = total_oscillators;
	oscillators[i] = new SynthOscillator(this, i);
	
	int y = i * OSCILLATORHEIGHT;
	if(thread) y -= thread->window->scroll->get_position();
	oscillators[i]->create_objects(y);
	total_oscillators++;
}

int Synth::delete_oscillator()
{
	if(total_oscillators)
	{
		delete oscillators[total_oscillators - 1];
		total_oscillators--;
	}
}


int Synth::destroy_oscillators()
{
	for(int i = 0 ; i < total_oscillators; i++)
	{
		delete oscillators[i];
	}
	total_oscillators = 0;
}



char* Synth::plugin_title() { return "Synthesizer"; }
int Synth::plugin_is_realtime() { return 1; }
int Synth::plugin_is_multi_channel() { return 0; }
int Synth::plugin_is_audio() { return 1; }

int Synth::start_realtime()
{
	redo_buffers = 1;
	dsp_buffer = 0;
	total_oscillators = 0;
	waveform_length = 0;
	samples_rendered = 0;
	srand(time(0));
}

int Synth::stop_realtime()
{
// don't delete main_in and main_out since they aren't arrays of pointers to buffers
	if(dsp_buffer) delete dsp_buffer;
	destroy_oscillators();
}

int Synth::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	main_in = input_ptr;
	main_out = output_ptr;
	if(redo_buffers) redo_buffers_procedure();
	
	long fragment_len;
	for(long i = 0; i < size; i += fragment_len)
	{
		fragment_len = size;
		if(i + fragment_len > size) fragment_len = size - i;
		
		fragment_len = overlay_synth(i, fragment_len);
	}
}

int Synth::overlay_synth(long start, long length)
{
	if(waveform_sample + length > waveform_length) length = waveform_length - waveform_sample;

// calculate some more data
// only calculate what's needed to speed it up
	if(waveform_sample + length > samples_rendered)
	{
		long start = waveform_sample, end = waveform_sample + length;
		for(int i = start; i < end; i++) dsp_buffer[i] = 0;
		solve_eqn(dsp_buffer, start, end);
		
		samples_rendered = end;
	}

	float *buffer_in = &main_in[start];
	float *buffer_out = &main_out[start];

	for(int i = 0; i < length; i++)
	{
		buffer_out[i] = buffer_in[i] + dsp_buffer[waveform_sample++];
	}
	if(waveform_sample >= waveform_length) waveform_sample = 0;
	
	return length;
}

int Synth::redo_buffers_procedure()
{
	if(dsp_buffer)
	{
		delete dsp_buffer;
	}

	waveform_length = project_sample_rate;
	period = (float)project_sample_rate / base_freq;
	dsp_buffer = new double[waveform_length + 1];

//printf("waveform_length %ld period %f\n", waveform_length, period);
	samples_rendered = 0;     // do some calculations on the next process_realtime
	redo_buffers = 0;
	waveform_sample = 0;
}

int Synth::solve_eqn(double *dsp_buffer, double x1, double x2)
{
	double normalize_constant = 1 / get_total_power();
	for(int i = 0; i < total_oscillators; i++)
		oscillators[i]->solve_eqn(dsp_buffer, x1, x2, normalize_constant);
}

double Synth::get_point(float x, double normalize_constant)
{
	double result = 0;
	for(int i = 0; i < total_oscillators; i++)
		result += oscillators[i]->get_point(x, normalize_constant);
	
	return result;
}

int Synth::start_gui()
{
	load_defaults();
	thread = new SynthThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start

	create_oscillators(total_oscillators);
	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->load_defaults(defaults);
	}
	update_gui();
}

int Synth::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->save_defaults(defaults);
	}
	save_defaults();
	destroy_oscillators();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int Synth::show_gui()
{
	thread->window->show_window();
}

int Synth::hide_gui()
{
	thread->window->hide_window();
}

int Synth::set_string()
{
	thread->window->set_title(gui_string);
}

int Synth::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("SYNTH");
	output.tag.set_property("BASEFREQ", base_freq);
	output.tag.set_property("WAVEFUNCTION", wavefunction);
	output.tag.set_property("OSCILLATORS", total_oscillators);
	output.append_tag();
	output.append_newline();

	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->save_data(&output);
	}

	output.terminate_string();
// data is now in *text
}

int Synth::read_data(char *text)
{
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));

	int result = 0, current_osc = 0;
	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("SYNTH"))
			{
				destroy_oscillators();
				base_freq = input.tag.get_property("BASEFREQ", base_freq);
				wavefunction = input.tag.get_property("WAVEFUNCTION", wavefunction);
				total_oscillators = input.tag.get_property("OSCILLATORS", 0);
				create_oscillators(total_oscillators);
			}
			else
			if(input.tag.title_is("OSCILLATOR"))
			{
				oscillators[current_osc]->read_data(&input);
				current_osc++;
			}
		}
	}
	redo_buffers = 1;
	update_gui();
}

int Synth::update_gui()
{
	if(thread)
	{
		thread->window->update_gui();
		for(int i = 0; i < total_oscillators; i++)
		{
			oscillators[i]->update_gui();
		}
	}
}

int Synth::reset()
{
	base_freq = 440;
	wavefunction = SINE;
	for(int i = 0; i < total_oscillators; i++)
	{
		oscillators[i]->reset();
	}
	update_gui();
}






int Synth::load_from_file(char *path)
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

int Synth::save_to_file(char *path)
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

int Synth::oscillator_height()
{
	return total_oscillators * OSCILLATORHEIGHT;
}

double Synth::get_total_power()
{
	double result = 0;
	for(int i = 0; i < total_oscillators; i++)
	{
		result += db.fromdb(oscillators[i]->level);
	}
	
	if(result == 0) result = 1;  // prevent division by 0
	return result;
}




SynthOscillator::SynthOscillator(Synth *synth, int number)
{
	this->number = number;
	this->synth = synth;
	level = number ? INFINITYGAIN : 0;
	phase = 0;
	freq_factor = 1;
	gui = 0;
}

SynthOscillator::~SynthOscillator()
{
	if(gui) delete gui;
}

int SynthOscillator::create_objects(int y)
{
	if(synth->thread)
	{
		gui = synth->thread->window->add_oscillator(this, y);
	}
}

int SynthOscillator::set_y(int position)
{
	if(gui)
	{
		gui->title->set_y(position + 15);
		gui->level->set_y(position);
		gui->phase->set_y(position);
		gui->freq->set_y(position);
	}
}

int SynthOscillator::update_gui()
{
	if(gui)
	{
		gui->level->update(level);
		gui->phase->update(phase * 360);
		gui->freq->update(freq_factor);
	}
}

int SynthOscillator::reset()
{
	if(number)
	{
		level = INFINITYGAIN;
	}
	else
	{
		level = 0;
	}

	phase = 0;
	freq_factor = 1;
}

int SynthOscillator::load_defaults(Defaults *defaults)
{
	char string[1024];
	
	sprintf(string, "LEVEL%d", number);
	level = defaults->get(string, INFINITYGAIN);
	sprintf(string, "PHASE%d", number);
	phase = defaults->get(string, 0);
	sprintf(string, "FREQFACTOR%d", number);
	freq_factor = defaults->get(string, 1);
}

int SynthOscillator::read_data(FileHTAL *input)
{
	level = input->tag.get_property("LEVEL", level);
	phase = input->tag.get_property("PHASE", phase);
	freq_factor = input->tag.get_property("FREQFACTOR", freq_factor);
}

int SynthOscillator::save_defaults(Defaults *defaults)
{
	char string[1024];
	
	sprintf(string, "LEVEL%d", number);
	defaults->update(string, level);
	sprintf(string, "PHASE%d", number);
	defaults->update(string, phase);
	sprintf(string, "FREQFACTOR%d", number);
	defaults->update(string, freq_factor);
}

int SynthOscillator::save_data(FileHTAL *output)
{
	output->tag.set_title("OSCILLATOR");
	output->tag.set_property("LEVEL", level);
	output->tag.set_property("PHASE", phase);
	output->tag.set_property("FREQFACTOR", freq_factor);
	output->append_tag();
	output->append_newline();
}


double SynthOscillator::solve_eqn(double *output, double x1, double x2, double normalize_constant)
{
	if(level <= INFINITYGAIN) return 0;

	double result;
	register double x;
	double power = synth->db.fromdb(level) * normalize_constant;
	double phase_offset = phase * synth->period;
	double x3 = x1 + phase_offset;
	double x4 = x2 + phase_offset;
	double period = synth->period / freq_factor;
	int sample;

	switch(synth->wavefunction)
	{
		case SINE:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += sin(x / period * 2 * M_PI) * power;
			}
			break;
		case SAWTOOTH:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += function_sawtooth(x / period) * power;
			}
			break;
		case SQUARE:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += function_square(x / period) * power;
			}
			break;
		case TRIANGLE:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += function_triangle(x / period) * power;
			}
			break;
		case PULSE:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += function_pulse(x / period) * power;
			}
			break;
		case NOISE:
			for(sample = (int)x1, x = x3; x < x4; x++, sample++)
			{
				output[sample] += function_noise() * power;
			}
			break;
	}
}

double SynthOscillator::function_square(double x)
{
	x -= (int)x; // only fraction counts
	return (x < .5) ? -1 : 1;
}

double SynthOscillator::function_pulse(double x)
{
	x -= (int)x; // only fraction counts
	return (x < .5) ? 0 : 1;
}

double SynthOscillator::function_noise()
{
	return (double)(rand() % 65536 - 32768) / 32768;
}

double SynthOscillator::function_sawtooth(double x)
{
	x -= (int)x;
	return 1 - x * 2;
}

double SynthOscillator::function_triangle(double x)
{
	x -= (int)x;
	return (x < .5) ? 1 - x * 4 : -3 + x * 4;
}

double SynthOscillator::get_point(float x, double normalize_constant)
{
	double power = synth->db.fromdb(level) * normalize_constant;
	switch(synth->wavefunction)
	{
		case SINE:
			return sin((x + phase) * freq_factor * 2 * M_PI) * power;
			break;
		case SAWTOOTH:
			return function_sawtooth((x + phase) * freq_factor) * power;
			break;
		case SQUARE:
			return function_square((x + phase) * freq_factor) * power;
			break;
		case TRIANGLE:
			return function_triangle((x + phase) * freq_factor) * power;
			break;
		case PULSE:
			return function_pulse((x + phase) * freq_factor) * power;
			break;
		case NOISE:
			return function_noise() * power;
			break;
	}
}
