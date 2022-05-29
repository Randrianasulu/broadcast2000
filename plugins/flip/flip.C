#include "filehtal.h"
#include "flip.h"
#include "flipwindow.h"

int main(int argc, char *argv[])
{
	FlipMain *plugin;

	plugin = new FlipMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

FlipMain::FlipMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	flip_vertical = 0;
	flip_horizontal = 0;
}

FlipMain::~FlipMain()
{
}

char* FlipMain::plugin_title() { return "Flip"; }
int FlipMain::plugin_is_realtime() { return 1; }
int FlipMain::plugin_is_multi_channel() { return 0; }
	
int FlipMain::start_realtime()
{
return 0;
}

int FlipMain::stop_realtime()
{
return 0;
}

int FlipMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k, l;
	VPixel **input_rows, **output_rows;
	VPixel *input_row, *output_row;

	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());

// Copy to new frame if necessary
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

		input_rows = output_rows;
		if(flip_vertical)
		{
			for(i = 0, j = project_frame_h - 1; i < project_frame_h / 2; i++, j--)
			{
				input_row = input_rows[i];
				output_row = output_rows[j];
				for(k = 0; k < project_frame_w; k++)
				{
					swap_pixels(&output_row[k], &input_row[k]);
				}
			}
		}

		if(flip_horizontal)
		{
			for(i = 0; i < project_frame_h; i++)
			{
				input_row = input_rows[i];
				output_row = output_rows[i];
				for(k = 0, l = project_frame_w - 1; k < project_frame_w / 2; k++, l--)
				{
					swap_pixels(&output_row[l], &input_row[k]);
				}
			}
		}
	}
return 0;
}

int FlipMain::swap_pixels(VPixel *in, VPixel *out)
{
	static VPixel temp;
	temp = *in;
	*in = *out;
	*out = temp;
return 0;
}


int FlipMain::start_gui()
{
	thread = new FlipThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int FlipMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int FlipMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int FlipMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int FlipMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int FlipMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	if(flip_vertical)
	{
		output.tag.set_title("VERTICAL");
		output.append_tag();
	}

	if(flip_horizontal)
	{	
		output.tag.set_title("HORIZONTAL");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
return 0;
}

int FlipMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	flip_vertical = flip_horizontal = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("VERTICAL"))
			{
				flip_vertical = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
			else
			if(input.tag.title_is("HORIZONTAL"))
			{
				flip_horizontal = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
		}
	}
	if(thread) 
	{
		thread->window->flip_vertical->update(flip_vertical);
		thread->window->flip_horizontal->update(flip_horizontal);
	}
return 0;
}
