#include "filehtal.h"
#include "invert.h"
#include "invertwindow.h"

int main(int argc, char *argv[])
{
	InvertMain *plugin;
	
	plugin = new InvertMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

InvertMain::InvertMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	invert = 1;
	thread = 0;
}

InvertMain::~InvertMain()
{
}

char* InvertMain::plugin_title() { return "Invert"; }
int InvertMain::plugin_is_realtime() { return 1; }
int InvertMain::plugin_is_multi_channel() { return 0; }
	
int InvertMain::start_realtime()
{
}

int InvertMain::stop_realtime()
{
}

int InvertMain::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	if(invert)
	for(register int j = 0; j < size; j++) output_ptr[j] = input_ptr[j] * -1;
	else
	for(register int j = 0; j < size; j++) output_ptr[j] = input_ptr[j];
}


int InvertMain::start_gui()
{
	thread = new InvertThread(this);
	thread->start();
	thread->gui_started.lock();
}

int InvertMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int InvertMain::show_gui()
{
	thread->window->show_window();
}

int InvertMain::hide_gui()
{
	thread->window->hide_window();
}

int InvertMain::set_string()
{
	thread->window->set_title(gui_string);
}

int InvertMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("INVERT");
	output.tag.set_property("VALUE", invert);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int InvertMain::read_data(char *text)
{
	FileHTAL input;
	
	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("INVERT"))
			{
				invert = input.tag.get_property("VALUE", invert);
			}
		}
	}
	if(thread) thread->window->toggle->update(invert);
}
