#include "filehtal.h"
#include "brightness.h"
#include "brightwindow.h"

int main(int argc, char *argv[])
{
	BrightnessMain *plugin;

	plugin = new BrightnessMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

BrightnessMain::BrightnessMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	brightness = 0;
	contrast = 0;
	defaults = 0;
}

BrightnessMain::~BrightnessMain()
{
	if(defaults) delete defaults;
}

char* BrightnessMain::plugin_title() { return "Brightness"; }
int BrightnessMain::plugin_is_realtime() { return 1; }
int BrightnessMain::plugin_is_multi_channel() { return 0; }
	
int BrightnessMain::start_realtime()
{
}

int BrightnessMain::stop_realtime()
{
}

int BrightnessMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k;
	VPixel **input_rows, **output_rows;
	int r, g, b;

//printf("BrightnessMain::process_realtime %f %f\n", brightness, contrast);


	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());


		if(brightness != 0)
		{
// Use int since brightness is also subtractive.
			int offset = (int)(brightness / 100 * VMAX);
#pragma omp parallel for private(i,j,k,r,g,b)  \
schedule(static) num_threads(4) collapse(2)

			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
					r = input_rows[j][k].r + offset;
					g = input_rows[j][k].g + offset;
					b = input_rows[j][k].b + offset;

					test_clip(r, g, b);

					output_rows[j][k].r = r;
					output_rows[j][k].g = g;
					output_rows[j][k].b = b;
					output_rows[j][k].a = input_rows[j][k].a;
				}
			}
// Data to be processed is now in the output buffer
			input_rows = output_rows;
		}

		if(contrast != 0)
		{
			int r, g, b, offset;
			float contrast_f = (contrast < 0) ? (contrast + 100) / 100 : (contrast + 50) / 50;

			if(use_float)
			{
// Floating point math.
				float scalar = contrast_f;
				float offset = VMAX / 2 - (VMAX * scalar) / 2;
#pragma omp parallel for private (j,k,r,g,b)  \
num_threads(4) collapse(2)
				for(j = 0; j < project_frame_h; j++)
				{
					for(k = 0; k < project_frame_w; k++)
					{
						r = (int)((float)input_rows[j][k].r * scalar + offset);
						g = (int)((float)input_rows[j][k].g * scalar + offset);
						b = (int)((float)input_rows[j][k].b * scalar + offset);

						test_clip(r, g, b);

						output_rows[j][k].r = r;
						output_rows[j][k].g = g;
						output_rows[j][k].b = b;
						output_rows[j][k].a = input_rows[j][k].a;
					}
				}
			}
			else
			{
// Integer math
				int r, g, b;
				int scalar = (int)(contrast_f * 0x100);
				int offset = VMAX * 0x100 / 2 - (VMAX * scalar) / 2;
#pragma omp parallel for private(j,k,r,g,b)  \
num_threads(4) collapse(2)
				for(j = 0; j < project_frame_h; j++)
				{
					for(k = 0; k < project_frame_w; k++)
					{
						r = (int)input_rows[j][k].r * scalar + offset;
						g = (int)input_rows[j][k].g * scalar + offset;
						b = (int)input_rows[j][k].b * scalar + offset;

						r >>= 8;
						g >>= 8;
						b >>= 8;
						test_clip(r, g, b);

						output_rows[j][k].r = r;
						output_rows[j][k].g = g;
						output_rows[j][k].b = b;
						output_rows[j][k].a = input_rows[j][k].a;
					}
				}
// Data to be processed is now in the output buffer
				input_rows = output_rows;
			}
		}

// Data never processed so copy if necessary
		if(!buffers_identical(0))
		{
//#pragma omp parallel for private(j,k)  \
//schedule(dynamic) collapse(2)

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

int BrightnessMain::test_clip(int &r, int &g, int &b)
{
	if(r > VMAX) r = VMAX;
	else
	if(r < 0) r = 0;

	if(g > VMAX) g = VMAX;
	else
	if(g < 0) g = 0;

	if(b > VMAX) b = VMAX;
	else
	if(b < 0) b = 0;

	return 0;
}


int BrightnessMain::start_gui()
{
	load_defaults();
	thread = new BrightThread(this);
	thread->start();
	thread->gui_started.lock();
}

int BrightnessMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int BrightnessMain::show_gui()
{
	thread->window->show_window();
}

int BrightnessMain::hide_gui()
{
	thread->window->hide_window();
}

int BrightnessMain::set_string()
{
	thread->window->set_title(gui_string);
}

int BrightnessMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sbrightness.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	brightness = defaults->get("BRIGHTNESS", (float)0);
	contrast = defaults->get("CONTRAST", (float)0);
}

int BrightnessMain::save_defaults()
{
	defaults->update("BRIGHTNESS", brightness);
	defaults->update("CONTRAST", contrast);
	defaults->save();
}

int BrightnessMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("BRIGHTNESS");
	output.tag.set_property("VALUE", brightness);
	output.append_tag();
	output.tag.set_title("CONTRAST");
	output.tag.set_property("VALUE", contrast);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int BrightnessMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("BRIGHTNESS"))
			{
				brightness = input.tag.get_property("VALUE", brightness);
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
			else
			if(input.tag.title_is("CONTRAST"))
			{
				contrast = input.tag.get_property("VALUE", contrast);
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
		}
	}
	if(thread) 
	{
		thread->window->bright_slider->update((int)brightness);
		thread->window->contrast_slider->update((int)contrast);
	}
}
