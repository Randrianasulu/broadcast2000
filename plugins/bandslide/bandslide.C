#include "filehtal.h"
#include "bandslide.h"
#include "bandslidewin.h"

int main(int argc, char *argv[])
{
	BandSlideMain *plugin;

	plugin = new BandSlideMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

BandSlideMain::BandSlideMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	total_bands = 7;
	reverse = 0;
	fake_input = 0;
}

BandSlideMain::~BandSlideMain()
{
}

const char* BandSlideMain::plugin_title() { return "BandSlide"; }
int BandSlideMain::plugin_is_realtime() { return 1; }
int BandSlideMain::plugin_is_multi_channel() { return 1; }

int BandSlideMain::start_realtime()
{
return 0;
}

int BandSlideMain::stop_realtime()
{
return 0;
}

int BandSlideMain::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
{
	VPixel **input1, **input2, **output;
	int i, j, k, left;
	 int in_x1, in_x2, out_x1, out_x2;
	int band_height, band_width;

// Want the top most layer to get the output.
// Use a blank frame for stripping if only 1 buffer is attached.
	if(total_in_buffers < 2)
	{
		if(!fake_input)
		{
			fake_input = new VFrame(0, project_frame_w, project_frame_h, VFRAME_VPIXEL);
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

	band_height = project_frame_h / total_bands;
	band_width = (int)(project_frame_w * ((float)get_source_position() / get_source_len()));

	for(i = 0, j = 0, left = 1; i < project_frame_h; left ^= 1, i += band_height)
	{
		if(i >= project_frame_h) i = project_frame_h - 1;

		for( ; j < i; j++)
		{
			if(left)
			{
				in_x1 = project_frame_w - band_width;
				in_x2 = project_frame_w;
				out_x1 = 0;
				out_x2 = band_width;

				while(in_x1 < in_x2)
				{
					output[j][out_x1] = input2[j][in_x1];
					in_x1++;
					out_x1++;
				}

				while(out_x1 < project_frame_w)
				{
					output[j][out_x1] = input1[j][out_x1];
					out_x1++;
				}
			}
			else
			{
				in_x1 = 0;
				in_x2 = band_width;
				out_x1 = 0;
				out_x2 = project_frame_w - band_width;

				while(out_x1 < out_x2)
				{
					output[j][out_x1] = input1[j][out_x1];
					out_x1++;
				}

				while(out_x1 < project_frame_w)
				{
					output[j][out_x1] = input2[j][in_x1];
					in_x1++;
					out_x1++;
				}
			}
		}
	}
return 0;
}



int BandSlideMain::start_gui()
{
	thread = new BandSlideThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int BandSlideMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int BandSlideMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int BandSlideMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int BandSlideMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int BandSlideMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);

// Store data
	output.tag.set_title("TOTALBANDS");
	output.tag.set_property("VALUE", total_bands);
	output.append_tag();

	if(reverse)
	{		
		output.tag.set_title("REVERSE");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
return 0;
}

int BandSlideMain::read_data(char *text)
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
			if(input.tag.title_is("TOTALBANDS"))
			{
				total_bands = input.tag.get_property("VALUE", total_bands);
			}
		}
	}
	if(thread) 
	{
		thread->window->total->update(total_bands);
		thread->window->reverse->update(reverse);
	}
return 0;
}
