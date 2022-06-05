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
 : PluginVClient(argc, argv)
{
	invert = 1;
}

InvertMain::~InvertMain()
{
}

const char* InvertMain::plugin_title() { return "Invert Video"; }
int InvertMain::plugin_is_realtime() { return 1; }
int InvertMain::plugin_is_multi_channel() { return 0; }

int InvertMain::start_realtime()
{
return 0;
}

int InvertMain::stop_realtime()
{
return 0;
}

int InvertMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i, j, k, l;
	VPixel **input_rows, **output_rows;
	VPixel *input_row, *output_row;

	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());

		if(invert)
		{
			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
					output_rows[j][k].r = VMAX - input_rows[j][k].r;
					output_rows[j][k].g = VMAX - input_rows[j][k].g;
					output_rows[j][k].b = VMAX - input_rows[j][k].b;
					output_rows[j][k].a = input_rows[j][k].a;
				}
			}
		}
		else
		if(input_rows != output_rows)
		{
			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
					output_rows[j][k] = input_rows[j][k];
				}
			}
		}
	}
return 0;
}


int InvertMain::start_gui()
{
	thread = new InvertThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int InvertMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int InvertMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int InvertMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int InvertMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int InvertMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	if(invert)
	{
		output.tag.set_title("INVERT");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
return 0;
}

int InvertMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	invert = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("INVERT"))
			{
				invert = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
		}
	}
	if(thread) 
	{
		thread->window->invert->update(invert);
	}
return 0;
}
