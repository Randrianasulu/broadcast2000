#include "filehtal.h"
#include "scale.h"
#include "scalewin.h"

int main(int argc, char *argv[])
{
	ScaleMain *plugin;

	plugin = new ScaleMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

ScaleMain::ScaleMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	scale_w = 1;
	scale_h = 1;
	constrain = 0;
	overlayer = 0;
	temp_frame = 0;
}

ScaleMain::~ScaleMain()
{
	if(overlayer) delete overlayer;
	if(temp_frame) delete temp_frame;
}

char* ScaleMain::plugin_title() { return "Scale"; }
int ScaleMain::plugin_is_realtime() { return 1; }
int ScaleMain::plugin_is_multi_channel() { return 0; }

int ScaleMain::start_realtime()
{
}

int ScaleMain::stop_realtime()
{
}

int ScaleMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	VFrame *input, *output;
	input = input_ptr[0];
	if(buffers_identical(0))
	{
		if(!temp_frame) temp_frame = new VFrame(0, project_frame_w, project_frame_h);
		output = temp_frame;
	}
	else
		output = output_ptr[0];

	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, use_interpolation, NORMAL))
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, use_interpolation, NORMAL);
	}

	if(scale_w == 1 && scale_h == 1)
	{
// No scaling
		if(!buffers_identical(0))
		{
			output_ptr[0]->copy_from(input_ptr[0]);
		}
	}
	else
	{
// Perform scaling
		float center_x, center_y;
		float in_x1, in_x2, in_y1, in_y2, out_x1, out_x2, out_y1, out_y2;

		center_x = (float)project_frame_w / 2;
		center_y = (float)project_frame_h / 2;
		in_x1 = 0;
		in_x2 = project_frame_w;
		in_y1 = 0;
		in_y2 = project_frame_h;
		out_x1 = (float)center_x - (float)project_frame_w * scale_w / 2;
		out_x2 = (float)center_x + (float)project_frame_w * scale_w / 2;
		out_y1 = (float)center_y - (float)project_frame_h * scale_h / 2;
		out_y2 = (float)center_y + (float)project_frame_h * scale_h / 2;

		if(out_x1 < 0)
		{
			in_x1 += -out_x1 / scale_w;
			out_x1 = 0;
		}

		if(out_x2 > project_frame_w)
		{
			in_x2 -= (out_x2 - project_frame_w) / scale_w;
			out_x2 = project_frame_w;
		}

		if(out_y1 < 0)
		{
			in_y1 += -out_y1 / scale_h;
			out_y1 = 0;
		}

		if(out_y2 > project_frame_h)
		{
			in_y2 -= (out_y2 - project_frame_h) / scale_h;
			out_y2 = project_frame_h;
		}

		output->clear_frame();

		overlayer->overlay(output, input,
			in_x1, in_y1, in_x2, in_y2,
			out_x1, out_y1, out_x2, out_y2, 
			VMAX);

		if(buffers_identical(0))
		{
			output_ptr[0]->copy_from(output);
		}
	}
}




int ScaleMain::start_gui()
{
	load_defaults();
	thread = new ScaleThread(this);
	thread->start();
	thread->gui_started.lock();
}

int ScaleMain::stop_gui()
{
	cleanup_gui();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int ScaleMain::cleanup_gui()
{
	save_defaults();
}

int ScaleMain::show_gui()
{
	thread->window->show_window();
}

int ScaleMain::hide_gui()
{
	thread->window->hide_window();
}

int ScaleMain::set_string()
{
	thread->window->set_title(gui_string);
}

int ScaleMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sscale.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	scale_w = defaults->get("WIDTH", (float)1);
	scale_h = defaults->get("HEIGHT", (float)1);
	constrain = defaults->get("CONSTRAIN", 1);
}

int ScaleMain::save_defaults()
{
	defaults->update("WIDTH", scale_w);
	defaults->update("HEIGHT", scale_h);
	defaults->update("CONSTRAIN", constrain);
	defaults->save();
}

int ScaleMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);

// Store data
	output.tag.set_title("SCALE");
	output.tag.set_property("WIDTH", scale_w);
	output.tag.set_property("HEIGHT", scale_h);
	output.append_tag();

	if(constrain)
	{
		output.tag.set_title("CONSTRAIN");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
}

int ScaleMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	constrain = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("SCALE"))
			{
				scale_w = input.tag.get_property("WIDTH", scale_w);
				scale_h = input.tag.get_property("HEIGHT", scale_h);
			}
			else
			if(input.tag.title_is("CONSTRAIN"))
			{
				constrain = 1;
			}
		}
	}
	if(thread) 
	{
		char string[1024];
		sprintf(string, "%.3f", scale_w);
		thread->window->width->update(string);
		sprintf(string, "%.3f", scale_h);
		thread->window->height->update(string);
		thread->window->constrain->update(constrain);
	}
}
