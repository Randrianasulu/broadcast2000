#include "filehtal.h"
#include "wipe.h"
#include "wipewin.h"

main(int argc, char *argv[])
{
	WipeMain *plugin;

	plugin = new WipeMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

WipeMain::WipeMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	direction = 0;
	reverse = 0;
	fake_input = 0;
}

WipeMain::~WipeMain()
{
}

char* WipeMain::plugin_title() { return "Wipe"; }
int WipeMain::plugin_is_realtime() { return 1; }
int WipeMain::plugin_is_multi_channel() { return 1; }

int WipeMain::start_realtime()
{
}

int WipeMain::stop_realtime()
{
}

int WipeMain::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
{
	VPixel **input1, **input2, **output;
	int i, j, k, left;
	register int in_x1, in_x2;
	int band_width;

// Want the top most layer to get the output.
// Use a blank frame for stripping if only 1 buffer is attached.
	if(total_in_buffers < 2)
	{
		if(!fake_input)
		{
			fake_input = new VFrame(0, project_frame_w, project_frame_h);
			fake_input->clear_frame();
		}
		input2 = (VPixel**)(reverse ? input_ptr[0][0]->get_rows() : fake_input->get_rows());
		input1 = (VPixel**)(reverse ? input_ptr[1][0]->get_rows() : input_ptr[0][0]->get_rows());
		output = (VPixel**)(output_ptr[0][0]->get_rows());
	}
	else
	{
		input2 = (VPixel**)(reverse ? input_ptr[0][0]->get_rows() : input_ptr[1][0]->get_rows());
		input1 = (VPixel**)(reverse ? input_ptr[1][0]->get_rows() : input_ptr[0][0]->get_rows());
		output = (VPixel**)(output_ptr[0][0]->get_rows());
	}

	band_width = (int)(project_frame_w * ((float)get_source_position() / get_source_len()));
	if(direction)
	{
// Move in from right.
		in_x2 = project_frame_w - band_width;

		for(i = 0; i < project_frame_h; i++)
		{
			in_x1 = 0;
			while(in_x1 < in_x2)
			{
				output[i][in_x1] = input1[i][in_x1];
				in_x1++;
			}

			while(in_x1 < project_frame_w)
			{
				output[i][in_x1] = input2[i][in_x1];
				in_x1++;
			}
		}
	}
	else
	{
// Move in from left
		in_x2 = band_width;
		
		for(i = 0; i < project_frame_h; i++)
		{
			in_x1 = 0;
			while(in_x1 < in_x2)
			{
				output[i][in_x1] = input2[i][in_x1];
				in_x1++;
			}
			
			while(in_x1 < project_frame_w)
			{
				output[i][in_x1] = input1[i][in_x1];
				in_x1++;
			}
		}
	}
}



int WipeMain::start_gui()
{
	thread = new WipeThread(this);
	thread->start();
	thread->gui_started.lock();
}

int WipeMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int WipeMain::show_gui()
{
	thread->window->show_window();
}

int WipeMain::hide_gui()
{
	thread->window->hide_window();
}

int WipeMain::set_string()
{
	thread->window->set_title(gui_string);
}

int WipeMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);

// Store data
	output.tag.set_title("DIRECTION");
	output.tag.set_property("RIGHT", direction);
	output.append_tag();

	if(reverse)
	{		
		output.tag.set_title("REVERSE");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
}

int WipeMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	reverse = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("REVERSE"))
			{
				reverse = 1;
			}
			else
			if(input.tag.title_is("DIRECTION"))
			{
				direction = input.tag.get_property("RIGHT", direction);
			}
		}
	}
	if(thread) 
	{
		thread->window->left->update(!direction);
		thread->window->right->update(direction);
		thread->window->reverse->update(reverse);
	}
}
