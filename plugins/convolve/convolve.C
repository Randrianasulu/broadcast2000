#include <math.h>
#include "convolve.h"
#include "filehtal.h"

int main(int argc, char *argv[])
{
	Convolve *plugin;

	plugin = new Convolve(argc, argv);
	plugin->plugin_run();
	delete plugin;
}


CrossfadeWindow::CrossfadeWindow()
{
	reset_crossfade();
}


CrossfadeWindow::~CrossfadeWindow()
{
	delete_crossfade();
}

int CrossfadeWindow::reset_crossfade()
{
	window_size = 4096;
	input_buffer = 0;
	
	return 0;
}

int CrossfadeWindow::delete_crossfade()
{
	int i;
	if(input_buffer)
	{
		for(i = 0; i < allocated_channels; i++)
		{
			delete [] dsp_in[i];
			delete [] dsp_out[i];
			delete [] input_buffer[i];
			delete [] output_buffer[i];
		}

		delete [] dsp_in;
		delete [] dsp_out;
		delete [] input_buffer;
		delete [] output_buffer;
	}
	input_buffer = 0;
	return 0;
}

int CrossfadeWindow::fix_window_size()
{
// fix the window size
// window size will be a power of 2
	int new_size = 16;
	while(new_size < window_size) new_size *= 2;
	window_size = new_size;
	return 0;
}

int CrossfadeWindow::init_crossfade(long fragment_size)
{
	this->fragment_size = fragment_size;
	first_window = 1;
	return 0;
}

long CrossfadeWindow::get_delay()
{
	return window_size + WINDOWBORDER;
}

int CrossfadeWindow::reconfigure(int new_channels)
{
	int new_size;
	int i, j;
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
	float **new_input_buffer = new float*[new_size];
	float **new_output_buffer = new float*[new_size];

// Create new buffer
	for(j = 0; j < new_channels; j++)
	{
		new_input_buffer[j] = new float[new_size];
		new_output_buffer[j] = new float[new_size];
		for(i = 0; i < new_size; i++)
		{
			new_input_buffer[j][i] = 0;
			new_output_buffer[j][i] = 0;
		}
	}

// Copy old buffer
	if(input_buffer)
	{
		for(j = 0; j < allocated_channels; j++)
		{
			for(i = 0; i < input_size; i++)
			{
				new_input_buffer[j][i] = input_buffer[j][i];
				new_output_buffer[j][i] = output_buffer[j][i];
			}
		}
	}

// Changing the window size greater than the fragment size will
// cause glitches since the fragment delay isn't adjusted here.
// Swap buffers
	delete_crossfade();
	input_size = new_size;
	allocated_channels = new_channels;
	dsp_in = new double*[new_channels];
	dsp_out = new double*[new_channels];
	input_buffer = new_input_buffer;
	output_buffer = new_output_buffer;

	for(j = 0; j < new_channels; j++)
	{
		dsp_in[j] = new double[window_size];
		dsp_out[j] = new double[window_size];
	}
	return 0;
}


int CrossfadeWindow::process_fifo(long size, 
		int new_channels, 
		float **output_ptr, 
		float **input_ptr)
{
	float *input_buffer_end;
	float *output_buffer_end;
	int i;

// copy the new fragments into the input_buffers
	for(i = 0; i < new_channels; i++)
	{
		 float *output = &input_buffer[i][fragment_in_position];
		 float *output_end = output + size;
		input_buffer_end = input_buffer[i] + input_size;

		if(output_end >= input_buffer_end) output_end -= input_size;

		while(output != output_end)
		{
			*output++ = *input_ptr[i]++;
			if(output >= input_buffer_end) output = input_buffer[i];
		}
	}

	fragment_in_position += size;
	if(fragment_in_position >= input_size) fragment_in_position -= input_size;

// Process windows
	while((fragment_in_position < window_position && input_size - window_position + fragment_in_position >= window_size) ||
		(fragment_in_position - window_position >= window_size))
	{
// copy a window from input_buffer to the dsp_in
		for(i = 0; i < new_channels; i++)
		{
			 float *input = &input_buffer[i][window_position];
			 float *input_end = input + window_size;
			double *output = dsp_in[i];

			input_buffer_end = input_buffer[i] + input_size;

			if(input_end > input_buffer_end) input_end -= input_size;

			while(input != input_end)
			{
				if(input >= input_buffer_end) input = input_buffer[i];
				*output++ = *input++;
			}
		}

// Do it
		signal_process(window_size, new_channels, dsp_out, dsp_in);

// copy from dsp_out into the output_buffer with crossfading
		for(i = 0; i < new_channels; i++)
		{
// crossfade first WINDOWBORDER with last WINDOWBORDER of previous window
			 float *output = &output_buffer[i][window_position];
			 float *output_end = output + WINDOWBORDER;
			 double *input = dsp_out[i];
			double input_slope = (WINDOWBORDER > 0) ? (double)1 / WINDOWBORDER : 0;
			float output_slope = (WINDOWBORDER > 0) ? (double)1 / WINDOWBORDER : 0;

			double scale = 0;
			output_buffer_end = output_buffer[i] + input_size;
			if(output_end > output_buffer_end) output_end -= input_size; 

// Don't crossfade first window.
// Crossfade in the first WINDOWBORDER
			if(first_window)
				while(output != output_end)
				{
					if(output >= output_buffer_end) output = output_buffer[i];
					*output++ = *input++;
				}
			else
				while(output != output_end)
				{
					if(output >= output_buffer_end) output = output_buffer[i];
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
				if(output >= output_buffer_end) output = output_buffer[i];
				*output++ = *input++;
			}
		}

// advance the window position
		window_position += window_size - WINDOWBORDER;
		if(window_position >= input_size) window_position -= input_size;
	}

	for(i = 0; i < new_channels; i++)
	{
// write the fragment from the oldest processed window to output
		float *output_end = output_ptr[i] + size;
		float *input;
		output_buffer_end = output_buffer[i] + input_size;

// Initially the fragment_out_position is < 0
		if(fragment_out_position < 0)
		{
			 float *new_output_end = output_end;

			if(fragment_out_position + size > 0) new_output_end -= fragment_out_position + size;
			while(output_ptr[i] < new_output_end)
			{
				*output_ptr[i]++ = 0;
			}
		}

		if(fragment_out_position < 0) 
			input = output_buffer[i];
		else
			input = &output_buffer[i][fragment_out_position];

		while(output_ptr[i] < output_end)
		{
			*output_ptr[i]++ = *input++;
			if(input >= output_buffer_end) input = output_buffer[i];
		}
	}

	fragment_out_position += size;
	if(fragment_out_position >= input_size) fragment_out_position -= input_size;
	return 0;
}




ConvolveSegment::ConvolveSegment(ConvolveEngine *engine)
 : Thread()
{
	synchronous = 1;
	this->engine = engine;
	done = 0;
}

ConvolveSegment::~ConvolveSegment()
{
	done = 1;
	input_lock.unlock();
	join();
}

int ConvolveSegment::start_thread()
{
	input_lock.lock();
	output_lock.lock();
	start();
	return 0;
}

int ConvolveSegment::process_range(long start, long end)
{
	this->buffer_start = start;
	this->buffer_end = end;
	input_lock.unlock();
	return 0;
}

int ConvolveSegment::wait_process()
{
	output_lock.lock();
	return 0;
}

void ConvolveSegment::run()
{
	while(!done)
	{
		input_lock.lock();
		if(done) return;

		for(int i = buffer_start, j = buffer_start + 1; i < buffer_end; i++, j++)
		{
			engine->temp[i] = dot_product(engine->input[0], 
							engine->chan_level[0], 
							&engine->input[1][i], 
							engine->chan_level[1], 
							j) / 64;
		}

		output_lock.unlock();
	}
}



ConvolveEngine::ConvolveEngine()
 : CrossfadeWindow()
{
	temp = 0;
	temp_size = 0;
	threads = 0;
	total_threads = 0;
}

ConvolveEngine::~ConvolveEngine()
{
	if(temp) delete [] temp;
	if(threads)
	{
		for(int i = 0; i < total_threads; i++)
		{
			delete threads[i];
		}
		delete [] threads;
	}
}

int ConvolveEngine::set_plugin(Convolve *plugin)
{
	this->plugin = plugin;
	if(!threads)
	{
		total_threads = (plugin->smp > 1) ? 2 : 1;
		threads = new ConvolveSegment*[total_threads];
		for(int i = 0; i < total_threads; i++)
		{
			threads[i] = new ConvolveSegment(this);
			threads[i]->start_thread();
		}
	}
	return 0;
}

int ConvolveEngine::signal_process(long size, int channels, double **output, double **input)
{
    	int i, j, k;
	double level_offset = 0;
	long fragment_size, master_fragment_size = (long)(size / total_threads + size / 4.5);
	DB db;

	this->output = output;
	this->input = input;
	chan_level[0] = plugin->chan_level[0];
	chan_level[1] = plugin->chan_level[1];
	if(plugin->automation_used())
	{
		level_offset += plugin->get_automation_value(position) * MAXOFFSET;
	}

	if(plugin->automated_level[0]) chan_level[0] += level_offset;
	if(plugin->automated_level[1]) chan_level[1] += level_offset;

	chan_level[0] = db.fromdb(chan_level[0]);
	chan_level[1] = db.fromdb(chan_level[1]);

// Just copy if single channel
	if(channels < 2)
	{
		for(i = 0; i < size; i++)
			output[0][i] = input[0][i];
	}
	else
	{
// Create temporary buffer
		if(size > temp_size && temp)
		{
			delete [] temp;
			temp = 0;
		}
		
		if(!temp)
		{
			temp = new double[size];
			temp_size = size;
		}

// ============================= Convolve the two channels ===========================
		for(i = 0; i < size; )
		{
			k = i;
			fragment_size = master_fragment_size;
			for(j = 0; j < total_threads && i < size; j++, i += fragment_size)
			{
				if(i + fragment_size > size) fragment_size = size - i;
				threads[j]->process_range(i, i + fragment_size);
			}

			i = k;
			fragment_size = master_fragment_size;
			for(j = 0; j < total_threads && i < size; j++, i += fragment_size)
			{
				if(i + fragment_size > size) fragment_size = size - i;
				threads[j]->wait_process();
			}
		}

		for(i = 0; i < size; i++)
		{
			output[0][i] = temp[i];
			output[1][i] = 0;
		}
	}

	for(j = 2; j < channels; j++)
		for(i = 0; i < size; i++)
			output[j][i] = 0;

	position += window_size;
	return 0;
}





Convolve::Convolve(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	thread = 0;

	reset_parameters();
}

Convolve::~Convolve()
{
}


int Convolve::reset_parameters()
{
// default parameters
	chan_level[0] = 0;
	chan_level[1] = 0;
	automated_level[0] = 0;
	automated_level[1] = 0;
	redo_buffers = 1;       // set to redo buffers before the first render
	engine.window_size = 16384;
	return 0;
}


int Convolve::redo_buffers_procedure()
{
	engine.reconfigure(total_in_buffers);
	redo_buffers = 0;
return 0;
}

int Convolve::load_defaults()
{
	char directory[1024];
// set the default directory
	sprintf(directory, "%sconvolve.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	chan_level[0] = defaults->get("LEVEL0", (float)0);
	chan_level[1] = defaults->get("LEVEL1", (float)1);
	automated_level[0] = defaults->get("AUTOLEVEL0", 1);
	automated_level[1] = defaults->get("AUTOLEVEL1", 0);
	engine.window_size = defaults->get("WINDOWSIZE", engine.window_size);
	return 0;
}

int Convolve::save_defaults()
{
	defaults->update("LEVEL0", chan_level[0]);
	defaults->update("LEVEL1", chan_level[1]);
	defaults->update("AUTOLEVEL0", automated_level[0]);
	defaults->update("AUTOLEVEL1", automated_level[1]);
	defaults->update("WINDOWSIZE", engine.window_size);
	defaults->save();
	return 0;
}

const char* Convolve::plugin_title() { return "Convolve"; }
int Convolve::plugin_is_realtime() { return 1; }
int Convolve::plugin_is_multi_channel() { return 1; }

int Convolve::start_realtime()
{
	engine.init_crossfade(in_buffer_size);
	redo_buffers_procedure();
return 0;
}

int Convolve::stop_realtime()
{
return 0;
}

int Convolve::process_realtime(long size, float **input_ptr, float **output_ptr)
{
	if(redo_buffers) redo_buffers_procedure();
	engine.set_plugin(this);
	engine.position = 0;
	engine.process_fifo(size, total_in_buffers, output_ptr, input_ptr);
	return 0;
}

int Convolve::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new ConvolveThread(this);
	thread->synchronous = 1;
	thread->start();
	thread->gui_started.lock();    // wait for the GUI to start

	update_gui();            // fill GUI with parameters
return 0;
}

int Convolve::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int Convolve::show_gui()
{
	thread->window->show_window();
return 0;
}

int Convolve::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int Convolve::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int Convolve::save_data(char *text)
{
	FileHTAL output;
// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("CONVOLVE");
	output.tag.set_property("LEVEL0", chan_level[0]);
	output.tag.set_property("LEVEL1", chan_level[1]);
	output.tag.set_property("AUTOLEVEL0", automated_level[0]);
	output.tag.set_property("AUTOLEVEL1", automated_level[1]);
	output.tag.set_property("WINDOWSIZE", engine.window_size);
	output.append_tag();
	output.append_newline();
	output.terminate_string();
// data is now in *text
return 0;
}

int Convolve::read_data(char *text)
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
			if(input.tag.title_is("CONVOLVE"))
			{
				chan_level[0] = input.tag.get_property("LEVEL0", chan_level[0]);
				chan_level[1] = input.tag.get_property("LEVEL1", chan_level[1]);
				automated_level[0] = input.tag.get_property("AUTOLEVEL0", automated_level[0]);
				automated_level[1] = input.tag.get_property("AUTOLEVEL1", automated_level[1]);
				engine.window_size = input.tag.get_property("WINDOWSIZE", engine.window_size);
			}
		}
	}

	redo_buffers = 1;
	update_gui();
return 0;
}

int Convolve::update_gui()
{
	if(thread)
	{
		thread->window->update_gui();
	}
return 0;
}

int Convolve::reset()
{
	reset_parameters();
	update_gui();
return 0;
}
