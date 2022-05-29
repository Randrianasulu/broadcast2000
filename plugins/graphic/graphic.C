#include <math.h>

#include "confirmsave.h"
#include "errorbox.h"
#include "graphic.h"
#include "filehtal.h"

int main(int argc, char *argv[])
{
	Graphic *plugin;

	plugin = new Graphic(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

GraphicEngine::GraphicEngine()
 : CrossfadeFFT()
{
}

GraphicEngine::~GraphicEngine()
{
}

int GraphicEngine::set_plugin(Graphic *graphic)
{
	this->graphic = graphic;
return 0;
}


int GraphicEngine::signal_process()
{
    	int h = window_size / 2;
    	int i;
	double magnitude, angle;

	for(i = 0; i < h; i++)
	{
		if(graphic->fft_coefs[i] != 1)
		{
			magnitude = graphic->fft_coefs[i] * sqrt(freq_real[i] * freq_real[i] + freq_imag[i] * freq_imag[i]);
			angle = atan2(freq_imag[i], freq_real[i]);
			freq_real[i] = magnitude * cos(angle);
			freq_imag[i] = magnitude * sin(angle);
		}
	}
	symmetry(window_size, freq_real, freq_imag);
return 0;
}



Graphic::Graphic(int argc, char *argv[]) : PluginAClient(argc, argv)
{
	defaults = 0;
	thread = 0;

	reset_parameters();
}

Graphic::~Graphic()
{
	delete_buffers();
	fourier.delete_fourier();
}


int Graphic::reset_parameters()
{
// default parameters
	for(int i = 0; i < MAXPOINTS; i++)
	{
		freqs[i] = 0;
		amounts[i] = 0;
	}

	total_points = 0;
	fft_coefs = 0;
	redo_buffers = 1;
	fourier.reset_fourier();
return 0;
}


int Graphic::init_buffers()
{
	fft_coefs = 0;
return 0;
}

int Graphic::delete_buffers()
{
	if(fft_coefs) delete fft_coefs;

	fft_coefs = 0;
return 0;
}

int Graphic::redo_buffers_procedure()
{
	long freq, i;
	fourier.reconfigure();
	delete_buffers();

	if(!fft_coefs) fft_coefs = new double[fourier.window_size / 2];
	for(i = 0; i < fourier.window_size / 2; i++)
	{
		fft_coefs[i] = get_coefficient((long)((float)i / fourier.window_size * project_sample_rate));
	}

	redo_buffers = 0;
return 0;
}

double Graphic::get_coefficient(long freq)
{
	long current_point;
	float result;
	float slope;

	if(freq == 0)
	{
		return 1;
	}
	else
	if(total_points == 0)
	{
		return 1;
	}
	else
	if(freq <= freqs[0])
	{
		slope = amounts[0] / freqs[0];
		result = freq * slope;
	}
	else
	if(freq > freqs[total_points - 1])
	{
		slope = -amounts[total_points - 1] / (project_sample_rate / 2 - freqs[total_points - 1]);
		result = (freq - freqs[total_points - 1]) * slope;
	}
	else
	{
		for(current_point = 1; 
			current_point < total_points && freq > freqs[current_point]; 
			current_point++)
		{ ; }

		slope = (amounts[current_point] - amounts[current_point - 1]) /
				(freqs[current_point] - freqs[current_point - 1]);
		result = amounts[current_point - 1] + (freq - freqs[current_point - 1]) * slope;
	}

	if(result <= -MAXLEVEL) result = INFINITYGAIN;
	return db.fromdb(result);
}


int Graphic::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sgraphic.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	sprintf(config_directory, BCASTDIR);
	defaults->get("CONFIG_DIRECTORY", config_directory);

	total_points = defaults->get("TOTALPOINTS", 0);
	for(int i = 0; i < total_points; i++)
	{
		sprintf(string, "FREQ%d", i);
		freqs[i] = defaults->get(string, 440);
		sprintf(string, "AMOUNT%d", i);
		amounts[i] = defaults->get(string, (float)1);
	}
	fourier.window_size = defaults->get("WINDOWSIZE", fourier.window_size);
return 0;
}

int Graphic::save_defaults()
{
	char string[1024];
	defaults->update("CONFIG_DIRECTORY", config_directory);

	defaults->update("TOTALPOINTS", total_points);
	for(int i = 0; i < total_points; i++)
	{
		sprintf(string, "FREQ%d", i);
		defaults->update(string, freqs[i]);
		sprintf(string, "AMOUNT%d", i);
		defaults->update(string, amounts[i]);
	}
	defaults->update("WINDOWSIZE", fourier.window_size);
	defaults->save();
return 0;
}

char* Graphic::plugin_title() { return "EQ Graphic"; }
int Graphic::plugin_is_realtime() { return 1; }
int Graphic::plugin_is_multi_channel() { return 0; }

int Graphic::start_realtime()
{
	init_buffers();
	fourier.init_fft(in_buffer_size);
	redo_buffers_procedure();
return 0;
}

int Graphic::stop_realtime()
{
	delete_buffers();
return 0;
}

int Graphic::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	if(redo_buffers) redo_buffers_procedure();
	fourier.set_plugin(this);
	fourier.process_fifo(size, input_ptr, output_ptr);
	return 0;
}

int Graphic::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new GraphicThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start

	update_gui();            // fill GUI with parameters
return 0;
}

int Graphic::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int Graphic::show_gui()
{
	thread->window->show_window();
return 0;
}

int Graphic::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int Graphic::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int Graphic::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("GRAPHICEQ");
	output.tag.set_property("TOTAL_POINTS", total_points);
	output.tag.set_property("WINDOWSIZE", fourier.window_size);
	output.append_tag();
	output.append_newline();

	for(int i = 0; i < total_points; i++)
	{
		output.tag.set_title("POINT");
		output.tag.set_property("FREQ", freqs[i]);
		output.tag.set_property("LEVEL", amounts[i]);
		output.append_tag();
		output.append_newline();
	}

	output.terminate_string();
// data is now in *text
return 0;
}

int Graphic::read_data(char *text)
{
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));

	int result = 0;
	int current_point = 0;
	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("GRAPHICEQ"))
			{
				total_points = input.tag.get_property("TOTAL_POINTS", total_points);
				fourier.window_size = input.tag.get_property("WINDOWSIZE", fourier.window_size);
			}
			else
			if(input.tag.title_is("POINT"))
			{
				freqs[current_point] = input.tag.get_property("FREQ", 0);
				amounts[current_point] = input.tag.get_property("LEVEL", (float)1);
				current_point++;
			}
		}
	}

	redo_buffers = 1;
	update_gui();
return 0;
}

int Graphic::update_gui()
{
	if(thread)
	{
		thread->window->canvas->draw_envelope();
		thread->window->windowsize->update((int)fourier.window_size);
	}
return 0;
}

int Graphic::reset()
{
	reset_parameters();
	update_gui();
return 0;
}


int Graphic::load_from_file(char *path)
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

int Graphic::save_to_file(char *path)
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
