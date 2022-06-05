#include "filehtal.h"
#include "delay.h"
#include "delaywindow.h"

int main(int argc, char *argv[])
{
	DelayMain *plugin;
	
	plugin = new DelayMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

DelayMain::DelayMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	duration = 100;
	thread = 0;
	dsp_in = 0;
}

DelayMain::~DelayMain()
{
}

const char* DelayMain::plugin_title() { return "Delay"; }
int DelayMain::plugin_is_realtime() { return 1; }
int DelayMain::plugin_is_multi_channel() { return 0; }
	
int DelayMain::start_realtime()
{
	dsp_in = new float[1];
	dsp_in[0] = 0;
	redo_buffers = 1;
	dsp_length = 0;
return 0;
}

int DelayMain::stop_realtime()
{
	delete dsp_in;
return 0;
}

int DelayMain::process_realtime(long size, float *input_ptr, float *output_ptr)
{
//printf("DelayMain::process_realtime 1\n");
	if(redo_buffers)
	{
		long new_dsp_length = in_buffer_size + duration;
		
		float *new_dsp = new float[new_dsp_length];
		 int i;
		for(i = 0; i < new_dsp_length && i < dsp_length; i++)
		{
			new_dsp[i] = dsp_in[i];
		}
		for( ; i < new_dsp_length; i++)
		{
			new_dsp[i] = 0;
		}
		if(dsp_in) delete dsp_in;
		dsp_in = new_dsp;
		dsp_length = new_dsp_length;
		redo_buffers = 0;
	}
//printf("DelayMain::process_realtime 2\n");

	int i, j;
// copy input to buffer since output is the input
	for(i = 0, j = dsp_length - in_buffer_size; i < size; i++, j++)
	{
		dsp_in[j] = input_ptr[i];
	}
// copy oldest dsp to output
	for(i = 0; i < size; i++)
	{
		output_ptr[i] = dsp_in[i];
	}
// shift dsp back
	for(j = 0; i < dsp_length; i++, j++)
	{
		dsp_in[j] = dsp_in[i];
	}
//printf("DelayMain::process_realtime 3\n");
return 0;
}


int DelayMain::start_gui()
{
	thread = new DelayThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int DelayMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int DelayMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int DelayMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int DelayMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int DelayMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("DELAY");
	output.tag.set_property("VALUE", duration);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int DelayMain::read_data(char *text)
{
	FileHTAL input;
	
	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("DELAY"))
			{
				duration = input.tag.get_property("VALUE", duration);
				redo_buffers = 1;
			}
		}
	}
	if(thread) thread->window->slider->update(duration);
return 0;
}
