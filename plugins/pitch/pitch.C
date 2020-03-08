#include <math.h>
#include "pitch.h"
#include "filehtal.h"

main(int argc, char *argv[])
{
	Pitch *plugin;

	plugin = new Pitch(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

PitchEngine::PitchEngine()
 : CrossfadeFFT()
{
//	temp_real = temp_imag = 0;
//	last_window = 0;
}
PitchEngine::~PitchEngine()
{
//	if(temp_real) delete temp_real;
//	if(temp_imag) delete temp_imag;
}

int PitchEngine::set_plugin(Pitch *pitch)
{
	this->pitch = pitch;
}

// int PitchEngine::update_buffers()
// {
// 	if(last_window != window_size / 2)
// 	{
// 		if(last_window)
// 		{
// 			delete temp_real;
// 			delete temp_imag;
// 		}
// 		
// 		last_window = window_size / 2 + 1;
// 		temp_real = new double[last_window];
// 		temp_imag = new double[last_window];
// 	}
// 	for(int i = 0; i < last_window; i++)
// 	{
// 		temp_real[i] = temp_imag[i] = 0;
// 	}
// }

int PitchEngine::signal_process()
{
    int h = window_size / 2;
	float freq_offset = pitch->freq_offset;
	double destination, upper, lower;
    register int i, dest_i, new_i;
	double freq_scale;

	if(pitch->automation_used())
	{
		freq_offset += pitch->get_automation_value(position) * MAXOFFSET;
	}

	freq_scale = (double)freq_offset / 100;
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

// Zero out the magnitude and phase
//	update_buffers();
//
// Overlay the source on the temp with interpolation
// Shift frequencies
//     for(i = 0; i <= h; i++)
//     {
// 		destination = i * freq_scale;
// 		dest_i = (int)(destination);
// 
// 		upper = floor(destination + 1) - destination;
// 		lower = destination - floor(destination);
// 
// 		if(dest_i < h)
// 		{
// 			temp_real[dest_i] += freq_real[i] * upper;
// 			temp_imag[dest_i] += freq_imag[i] * upper;
// 			dest_i++;
// 			temp_real[dest_i] += freq_real[i] * lower;
// 			temp_imag[dest_i] += freq_imag[i] * lower;
// 		}
//    }
//
// Move temporary frequencies to output
// 	for(i = 0; i <= h; i++)
// 	{
// 		freq_real[i] = temp_real[i];
// 		freq_imag[i] = temp_imag[i];
// 	}

	symmetry(window_size, freq_real, freq_imag);
	position += window_size;
}





Pitch::Pitch(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	thread = 0;

	reset_parameters();
}

Pitch::~Pitch()
{
}


int Pitch::reset_parameters()
{
// default parameters
	freq_offset = 100;
	redo_buffers = 1;       // set to redo buffers before the first render
	engine.window_size = 16384;
}


int Pitch::redo_buffers_procedure()
{
	engine.reconfigure();
	redo_buffers = 0;
}

int Pitch::load_defaults()
{
	char directory[1024];
// set the default directory
	sprintf(directory, "%spitch.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	freq_offset = defaults->get("OFFSET", 100);
	engine.window_size = defaults->get("WINDOWSIZE", engine.window_size);
}

int Pitch::save_defaults()
{
	defaults->update("OFFSET", freq_offset);
	defaults->update("WINDOWSIZE", engine.window_size);
	defaults->save();
}

char* Pitch::plugin_title() { return "Pitch"; }
int Pitch::plugin_is_realtime() { return 1; }
int Pitch::plugin_is_multi_channel() { return 0; }

int Pitch::start_realtime()
{
	engine.init_fft(in_buffer_size);
	redo_buffers_procedure();
}

int Pitch::stop_realtime()
{
}

int Pitch::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	if(redo_buffers) redo_buffers_procedure();
	engine.set_plugin(this);
	engine.position = 0;
	engine.process_fifo(size, input_ptr, output_ptr);
	return 0;
}

int Pitch::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new PitchThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start

	update_gui();            // fill GUI with parameters
}

int Pitch::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int Pitch::show_gui()
{
	thread->window->show_window();
}

int Pitch::hide_gui()
{
	thread->window->hide_window();
}

int Pitch::set_string()
{
	thread->window->set_title(gui_string);
}

int Pitch::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("PITCH");
	output.tag.set_property("OFFSET", freq_offset);
	output.tag.set_property("WINDOWSIZE", engine.window_size);
	output.append_tag();
	output.append_newline();
	output.terminate_string();
// data is now in *text
}

int Pitch::read_data(char *text)
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
			if(input.tag.title_is("PITCH"))
			{
				freq_offset = input.tag.get_property("OFFSET", freq_offset);
				engine.window_size = input.tag.get_property("WINDOWSIZE", engine.window_size);
			}
		}
	}

	redo_buffers = 1;
	update_gui();
}

int Pitch::update_gui()
{
	if(thread)
	{
		thread->window->update_gui();
	}
}

int Pitch::reset()
{
	reset_parameters();
	update_gui();
}
