#include "filehtal.h"
#include "barndoor.h"
#include "barndoorwin.h"

int main(int argc, char *argv[])
{
	BarnDoorMain *plugin;

	plugin = new BarnDoorMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

BarnDoorMain::BarnDoorMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	reverse = 0;
	fake_input = 0;
}

BarnDoorMain::~BarnDoorMain()
{
}

char* BarnDoorMain::plugin_title() { return "BarnDoor"; }
int BarnDoorMain::plugin_is_realtime() { return 1; }
int BarnDoorMain::plugin_is_multi_channel() { return 1; }

int BarnDoorMain::start_realtime()
{
return 0;
}

int BarnDoorMain::stop_realtime()
{
return 0;
}

int BarnDoorMain::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
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
	in_x1 = project_frame_w / 2 - band_width / 2;
	in_x2 = project_frame_w / 2 + band_width / 2;
	
	if(in_x1 < 0) in_x1 = 0;
	if(in_x2 > project_frame_w) in_x2 = project_frame_w;


	for(i = 0; i < project_frame_h; i++)
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
return 0;
}



int BarnDoorMain::start_gui()
{
	thread = new BarnDoorThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int BarnDoorMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int BarnDoorMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int BarnDoorMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int BarnDoorMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int BarnDoorMain::save_data(char *text)
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
return 0;
}

int BarnDoorMain::read_data(char *text)
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
return 0;
}
