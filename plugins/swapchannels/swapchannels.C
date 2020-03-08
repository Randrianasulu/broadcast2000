#include "filehtal.h"
#include "swapchannels.h"

main(int argc, char *argv[])
{
	SwapMain *plugin;

	plugin = new SwapMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

SwapMain::SwapMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	red = RED_SRC;
	green = GREEN_SRC;
	blue = BLUE_SRC;
    alpha = ALPHA_SRC;
}

SwapMain::~SwapMain()
{
}

char* SwapMain::plugin_title() { return "Swap channels"; }
int SwapMain::plugin_is_realtime() { return 1; }
int SwapMain::plugin_is_multi_channel() { return 0; }
	
int SwapMain::start_realtime()
{
}

int SwapMain::stop_realtime()
{
}

#define MAXMINSRC(src) \
	(src == MAX_SRC ? VMAX : 0)

int SwapMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k;
	VPixel **input_rows, **output_rows;
	VPixel input_pixel;
	register VWORD r, g, b, a;

	for(i = 0; i < size; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();

		if(red != RED_SRC || green != GREEN_SRC || blue != BLUE_SRC || alpha != ALPHA_SRC)
		{
			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
// Assume the order of channels
					input_pixel = input_rows[j][k];

					if(red < 4)
						output_rows[j][k].r = *((VWORD*)&input_pixel + red);
					else
						output_rows[j][k].r = MAXMINSRC(red);

					if(green < 4)
						output_rows[j][k].g = *((VWORD*)&input_pixel + green);
					else
						output_rows[j][k].g = MAXMINSRC(green);

					if(blue < 4)
						output_rows[j][k].b = *((VWORD*)&input_pixel + blue);
					else
						output_rows[j][k].b = MAXMINSRC(blue);

					if(alpha < 4)
						output_rows[j][k].a = *((VWORD*)&input_pixel + alpha);
					else
						output_rows[j][k].a = MAXMINSRC(alpha);
				}
			}
		}
		else
// Data never processed so copy if necessary
		if(!buffers_identical(0))
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
}


int SwapMain::start_gui()
{
	load_defaults();
	thread = new SwapThread(this);
	thread->start();
	thread->gui_started.lock();
}

int SwapMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int SwapMain::show_gui()
{
	thread->window->show_window();
}

int SwapMain::hide_gui()
{
	thread->window->hide_window();
}

int SwapMain::set_string()
{
	thread->window->set_title(gui_string);
}

int SwapMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sswapchannels.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	red = defaults->get("RED", red);
	green = defaults->get("GREEN", green);
	blue = defaults->get("BLUE", blue);
	alpha = defaults->get("ALPHA", alpha);
}

int SwapMain::save_defaults()
{
	defaults->update("RED", red);
	defaults->update("GREEN", green);
	defaults->update("BLUE", blue);
	defaults->update("ALPHA", alpha);
	defaults->save();
}

int SwapMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("SWAPCHANNELS");
	output.tag.set_property("RED", red);
	output.tag.set_property("GREEN", green);
	output.tag.set_property("BLUE", blue);
	output.tag.set_property("ALPHA", alpha);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int SwapMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("SWAPCHANNELS"))
			{
				red = input.tag.get_property("RED", red);
				green = input.tag.get_property("GREEN", green);
				blue = input.tag.get_property("BLUE", blue);
				alpha = input.tag.get_property("ALPHA", alpha);
			}
		}
	}
	if(thread) 
	{
		thread->window->red->update(output_to_text(red));
		thread->window->green->update(output_to_text(green));
		thread->window->blue->update(output_to_text(blue));
		thread->window->alpha->update(output_to_text(alpha));
	}
}

char* SwapMain::output_to_text(int value)
{
	switch(value)
	{
		case RED_SRC:
			return "Red";
			break;
		case GREEN_SRC:
			return "Green";
			break;
		case BLUE_SRC:
			return "Blue";
			break;
		case ALPHA_SRC:
			return "Alpha";
			break;
		case NO_SRC:
			return "0%";
			break;
		case MAX_SRC:
			return "100%";
			break;
		default:
			return "";
			break;
	}
}

int SwapMain::text_to_output(char *text)
{
	if(!strcmp(text, "Red")) return RED_SRC;
	if(!strcmp(text, "Green")) return GREEN_SRC;
	if(!strcmp(text, "Blue")) return BLUE_SRC;
	if(!strcmp(text, "Alpha")) return ALPHA_SRC;
	if(!strcmp(text, "0%")) return NO_SRC;
	if(!strcmp(text, "100%")) return MAX_SRC;
	return 0;
}


