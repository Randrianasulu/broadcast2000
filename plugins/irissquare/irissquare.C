#include "filehtal.h"
#include "irissquare.h"

main(int argc, char *argv[])
{
	IrisSquareMain *plugin;

	plugin = new IrisSquareMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

IrisSquareMain::IrisSquareMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	reverse = 0;
	fake_input = 0;
}

IrisSquareMain::~IrisSquareMain()
{
}

char* IrisSquareMain::plugin_title() { return "IrisSquare"; }
int IrisSquareMain::plugin_is_realtime() { return 1; }
int IrisSquareMain::plugin_is_multi_channel() { return 1; }

int IrisSquareMain::start_realtime()
{
}

int IrisSquareMain::stop_realtime()
{
}

int IrisSquareMain::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
{
	VPixel **input1, **input2, **output;
	int i, j, k, left;
	register int in_x1, in_x2, in_y1, in_y2;
	int band_width, band_height;

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
		output = ((VPixel**)output_ptr[0][0]->get_rows());
	}
	else
	{
		input2 = (VPixel**)(reverse ? input_ptr[0][0]->get_rows() : input_ptr[1][0]->get_rows());
		input1 = (VPixel**)(reverse ? input_ptr[1][0]->get_rows() : input_ptr[0][0]->get_rows());
		output = ((VPixel**)output_ptr[0][0]->get_rows());
	}

	band_width = (int)(project_frame_w * ((float)get_source_position() / get_source_len()));
	band_height = (int)(project_frame_h * ((float)get_source_position() / get_source_len()));

	in_x1 = project_frame_w / 2 - band_width / 2;
	in_x2 = project_frame_w / 2 + band_width / 2;
	in_y1 = project_frame_h / 2 - band_height / 2;
	in_y2 = project_frame_h / 2 + band_height / 2;

	if(in_x1 < 0) in_x1 = 0;
	if(in_x2 > project_frame_w) in_x2 = project_frame_w;
	if(in_y1 < 0) in_y1 = 0;
	if(in_y2 > project_frame_h) in_y2 = project_frame_h;

	for(i = 0; i < in_y1; i++)
	{
		for(j = 0; j < project_frame_w; j++)
		{
			output[i][j] = input1[i][j];
		}
	}
	for( ; i < in_y2; i++)
	{
		for(j = 0; j < in_x1; j++)
		{
			output[i][j] = input1[i][j];
		}
		for( ; j < in_x2; j++)
		{
			output[i][j] = input2[i][j];
		}
		for( ; j < project_frame_w; j++)
		{
			output[i][j] = input1[i][j];
		}
	}
	for( ; i < project_frame_h; i++)
	{
		for(j = 0; j < project_frame_w; j++)
		{
			output[i][j] = input1[i][j];
		}
	}
}



int IrisSquareMain::start_gui()
{
	thread = new IrisSquareThread(this);
	thread->start();
	thread->gui_started.lock();
}

int IrisSquareMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int IrisSquareMain::show_gui()
{
	thread->window->show_window();
}

int IrisSquareMain::hide_gui()
{
	thread->window->hide_window();
}

int IrisSquareMain::set_string()
{
	thread->window->set_title(gui_string);
}

int IrisSquareMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);

// Store data
	if(reverse)
	{		
		output.tag.set_title("REVERSE");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
}

int IrisSquareMain::read_data(char *text)
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
		}
	}
	if(thread) 
	{
		thread->window->reverse->update(reverse);
	}
}
