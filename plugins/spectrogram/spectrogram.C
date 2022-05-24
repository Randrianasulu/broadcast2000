#include <math.h>
#include "spectrogram.h"
#include "filehtal.h"
#include "vframe.h"

int main(int argc, char *argv[])
{
	Spectrogram *plugin;

	plugin = new Spectrogram(argc, argv);
	plugin->plugin_run();
	delete plugin;
}





Spectrogram::Spectrogram(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	thread = 0;
	trigger = 0;
	magnitude = 0;
	trigger_id = -1;
	magnitudes_id = -1;
	trigger_thread = 0;
	freq_real = 0;
	freq_imag = 0;
	fft_in = 0;

	reset_parameters();
}

Spectrogram::~Spectrogram()
{
	if(magnitude) delete magnitude;
	if(trigger)
	{
		trigger->write_message_flagged(TO_GUI, COMPLETED);
		delete trigger;
	}
	if(trigger_thread) 
	{
		trigger_thread->join();
		delete trigger_thread;
	}
	if(fft_in) delete fft_in;
	if(freq_real) delete freq_real;
	if(freq_imag) delete freq_imag;
}


int Spectrogram::reset_parameters()
{
// default parameters
	redo_buffers = 1;       // set to redo buffers before the first render
	window_size = 4096;
	allocated_size = 0;
	w = 320;
	h = 200;
}


int Spectrogram::redo_buffers_procedure()
{
	if(trigger) delete trigger;
	trigger = 0;

// Create shared objects only if there's a GUI
	if(master_gui_on)
	{
		if(trigger_id != -1) trigger = new Messages(TO_DSP, TO_GUI, trigger_id);
	}
	else
	{
		trigger_id = -1;
	}

	redo_buffers = 0;
}

int Spectrogram::get_canvas_w()
{
	return w - 20;
}

int Spectrogram::get_canvas_h()
{
	return h - 70;
}

int Spectrogram::load_defaults()
{
	char directory[1024];
// set the default directory
	sprintf(directory, "%sspectrogram.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	w = defaults->get("WIDTH", w);
	h = defaults->get("HEIGHT", h);
	window_size = defaults->get("WINDOWSIZE", window_size);
}

int Spectrogram::save_defaults()
{
	defaults->update("WIDTH", w);
	defaults->update("HEIGHT", h);
	defaults->update("WINDOWSIZE", window_size);
	defaults->save();
}

char* Spectrogram::plugin_title() { return "Spectrogram"; }
int Spectrogram::plugin_is_realtime() { return 1; }
int Spectrogram::plugin_is_multi_channel() { return 1; }

int Spectrogram::start_realtime()
{
	SharedMem *test;
	allocated_size = max_window_size(in_buffer_size);
	test = new SharedMem((allocated_size + 2) * sizeof(double));
	magnitude = test;
	fft_in = new double[allocated_size];
	freq_real = new double[allocated_size];
	freq_imag = new double[allocated_size];

	redo_buffers = 1;
}

int Spectrogram::stop_realtime()
{
	if(trigger) delete trigger;
	if(magnitude) delete magnitude;
	if(fft_in) delete fft_in;
	if(freq_real) delete freq_real;
	if(freq_imag) delete freq_imag;
	trigger = 0;
	magnitude = 0;
	fft_in = 0;
	freq_real = 0;
	freq_imag = 0;
	allocated_size = 0;
	trigger_id = -1;
	magnitudes_id = -1;
}

int Spectrogram::max_window_size(int fragment_size)
{
	int result;

	for(result = 2; result <= fragment_size; result *= 2)
		;
	result /= 2;

	return result;
}


int Spectrogram::process_realtime(long size, float **input_ptr, float **output_ptr)
{
	register int i, j, k;
	int real_window, result;

// Pass buffers through if necessary
	for(i = 0; i < total_in_buffers; i++)
	{
		if(input_ptr[i] != output_ptr[i])
		{
			for(j = 0; j < size; j++) output_ptr[i][j] = input_ptr[i][j];
		}
	}

	if(redo_buffers) redo_buffers_procedure();

	if(master_gui_on)
	{
    	int h;

// Get a window size based on the fragment size
		for(real_window = 2; real_window <= window_size && real_window <= size; real_window *= 2)
			;
		real_window /= 2;

		for(i = 0; i < real_window; i++) fft_in[i] = input_ptr[0][i];

// Do the fft
		fft.do_fft(real_window,  // must be a power of 2
    		0,         // 0 = forward FFT, 1 = inverse
    		fft_in,     // array of input's real samples
    		0,     // array of input's imag samples
    		freq_real,    // array of output's reals
    		freq_imag);

		h = real_window / 2;

// Save magnitudes
		((double*)magnitude->data)[0] = h;
		for(i = 0; i < h; i++)
		{
			((double*)magnitude->data)[i + 1] = sqrt(freq_real[i] * freq_real[i] + freq_imag[i] * freq_imag[i]);
		}
		if(trigger)
		{
			trigger->write_message(UPDATE_GUI);   // Send trigger
			trigger->write_message(magnitude->get_id(), allocated_size + 2, project_sample_rate);  // Send shm id
			result = trigger->read_message();  // Wait for completion or error
		}
	}

	return 0;
}

int Spectrogram::load_bitmap(VFrame *bitmap)
{
	int i, j;
	long magnitudes_id, magnitudes_size;
	int total_magnitudes;
	int index1, index2, freq1, freq2, frequency;
	int pixel1, pixel2;
	float freq_fraction;
	int fft_entry;
	int top, bottom;
	float scale;
	long sample_rate;

	trigger->read_message(&magnitudes_id, &magnitudes_size, &sample_rate);
	magnitude = new SharedMem(magnitudes_id, magnitudes_size * sizeof(double));
	total_magnitudes = (int)(((double*)magnitude->data)[0]);
	scale = (float)bitmap->get_h() / 256;

	bottom = bitmap->get_h();
	for(i = 0; i < bitmap->get_w(); i++)
	{
// Get boundary frequency indexes of pixel
		index1 = (int)((float)i / bitmap->get_w() * TOTALFREQS);
		index2 = (int)((float)i / bitmap->get_w() * TOTALFREQS) + 1;
 		if(index1 >= TOTALFREQS) index1 = TOTALFREQS - 1;
 		if(index2 >= TOTALFREQS) index2 = TOTALFREQS - 1;
// Get actual frequency of the pixel
 		freq1 = freq_table.tofreq(index1);
 		freq2 = freq_table.tofreq(index2);
		if(freq1 == freq2) 
			frequency = freq1;
		else
			frequency = (int)(((float)i / bitmap->get_w() * TOTALFREQS - index1) * (freq2 - freq1) + freq1);

// Cut off below 50
		frequency += 50;
		fft_entry = (int)((float)frequency / (freq_table.tofreq(TOTALFREQS - 1)) * total_magnitudes);
		if(fft_entry >= total_magnitudes) fft_entry = total_magnitudes - 1;
// Get top and bottom of line
		top = (int)(bitmap->get_h() - scale * ((double*)magnitude->data)[fft_entry]);
		if(top < 0) top = 0;
		if(top > bitmap->get_h()) top = bitmap->get_h();

		for(j = 0; j < top; j++)
		{
			((VPixel**)bitmap->get_rows())[j][i].r = 0;
			((VPixel**)bitmap->get_rows())[j][i].g = 0;
			((VPixel**)bitmap->get_rows())[j][i].b = 0;
			((VPixel**)bitmap->get_rows())[j][i].a = 0;
		}

		for( ; j < bottom; j++)
		{
			((VPixel**)bitmap->get_rows())[j][i].r = VMAX;
			((VPixel**)bitmap->get_rows())[j][i].g = VMAX;
			((VPixel**)bitmap->get_rows())[j][i].b = VMAX;
			((VPixel**)bitmap->get_rows())[j][i].a = VMAX;
		}
	}
	delete magnitude;
	trigger->write_message(COMPLETED);
}


int Spectrogram::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	if(!trigger) trigger = new Messages(TO_GUI, TO_DSP);
	trigger_id = trigger->get_id();
	thread = new SpectrogramThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start
	trigger_thread = new SpectrogramTrigger(this, thread->window);
	trigger_thread->start();
	trigger_thread->startup_lock.lock();

	update_gui();             // fill GUI with parameters
	send_configure_change();  // Propogate new configuration
}

int Spectrogram::stop_gui()
{
	cleanup_gui();
// defaults only need to be saved when the GUI is closed
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int Spectrogram::cleanup_gui()
{
	magnitudes_id = -1;
	trigger_id = -1;
	allocated_size = 0;
	send_configure_change();  // Propogate new configuration

	if(trigger_thread)
	{
		trigger->write_message_flagged(TO_GUI, COMPLETED);
		trigger_thread->join();
		delete trigger_thread;
	}

	if(trigger) delete trigger;
	save_defaults();
	trigger = 0;
	trigger_thread = 0;
}

int Spectrogram::show_gui()
{
	thread->window->show_window();
}

int Spectrogram::hide_gui()
{
	thread->window->hide_window();
}

int Spectrogram::set_string()
{
	thread->window->set_title(gui_string);
}

int Spectrogram::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("SPECTROGRAM");
	output.tag.set_property("WIDTH", w);
	output.tag.set_property("HEIGHT", h);
	output.tag.set_property("WINDOWSIZE", window_size);
	output.tag.set_property("TRIGGER", trigger_id);
	output.append_tag();
	output.append_newline();
	output.terminate_string();
// data is now in *text
}

int Spectrogram::read_data(char *text)
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
			if(input.tag.title_is("SPECTROGRAM"))
			{
				w = input.tag.get_property("WIDTH", w);
				h = input.tag.get_property("HEIGHT", h);
				window_size = input.tag.get_property("WINDOWSIZE", window_size);
// Only load ids if not the GUI plugin
				if(!thread)
				{
					trigger_id = input.tag.get_property("TRIGGER", -1);
				}
			}
		}
	}

	redo_buffers = 1;
	update_gui();
}

int Spectrogram::update_gui()
{
	if(thread)
	{
		thread->window->update_gui();
	}
}

int Spectrogram::reset()
{
	reset_parameters();
	update_gui();
}
