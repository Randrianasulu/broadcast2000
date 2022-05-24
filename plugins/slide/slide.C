#include "filehtal.h"
#include "slide.h"
#include "slidewin.h"

int main(int argc, char *argv[])
{
	SlideMain *plugin;

	plugin = new SlideMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

SlideMain::SlideMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	direction = 0;
	reverse = 0;
	fake_input = 0;
}

SlideMain::~SlideMain()
{
}

char* SlideMain::plugin_title() { return "Slide"; }
int SlideMain::plugin_is_realtime() { return 1; }
int SlideMain::plugin_is_multi_channel() { return 1; }

int SlideMain::start_realtime()
{
}

int SlideMain::stop_realtime()
{
}

int SlideMain::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
{
	VPixel **input1, **input2, **output;
	int i, j, k, left;
	register int in_x1, in_x2, out_x1, out_x2;
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
		output = (VPixel**)output_ptr[0][0]->get_rows();
	}
	else
	{
		input2 = (VPixel**)(reverse ? input_ptr[0][0]->get_rows() : input_ptr[1][0]->get_rows());
		input1 = (VPixel**)(reverse ? input_ptr[1][0]->get_rows() : input_ptr[0][0]->get_rows());
		output = (VPixel**)output_ptr[0][0]->get_rows();
	}

	band_width = (int)(project_frame_w * ((float)get_source_position() / get_source_len()));
	if(direction)
	{
// Move in from right.
		out_x1 = project_frame_w - band_width;
		out_x2 = project_frame_w;

		for(i = 0; i < project_frame_h; i++)
		{
			for(k = 0; k < out_x1; k++)
			{
				output[i][k] = input1[i][k];
			}

			for(j = 0; j < band_width; j++, k++)
			{
				output[i][k] = input2[i][j];
			}
		}
	}
	else
	{
// Move in from left
		in_x1 = project_frame_w - band_width;
		in_x2 = project_frame_w;
		
		for(i = 0; i < project_frame_h; i++)
		{
			for(j = 0, k = in_x1; j < band_width; j++, k++)
			{
				output[i][j] = input2[i][k];
			}
			
			for( ; j < project_frame_w; j++)
			{
				output[i][j] = input1[i][j];
			}
		}
	}
}



int SlideMain::start_gui()
{
	thread = new SlideThread(this);
	thread->start();
	thread->gui_started.lock();
}

int SlideMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int SlideMain::show_gui()
{
	thread->window->show_window();
}

int SlideMain::hide_gui()
{
	thread->window->hide_window();
}

int SlideMain::set_string()
{
	thread->window->set_title(gui_string);
}

int SlideMain::save_data(char *text)
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

int SlideMain::read_data(char *text)
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
