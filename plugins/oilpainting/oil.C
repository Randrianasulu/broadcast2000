#include <math.h>
#include "filehtal.h"
#include "oil.h"
#include "oilwindow.h"


// Algorithm by Torsten Martinsen
// Ported to Broadcast by Adam Williams

#define HISTSIZE	VMAX + 1
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define INTENSITY(p) ((unsigned int)((p.r) * 77+ \
									(p.g * 150) + \
									(p.b * 29)) >> 8)

int main(int argc, char *argv[])
{
	OilMain *plugin;

	plugin = new OilMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

OilMain::OilMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	radius = 7;
	use_intensity = 0;
	defaults = 0;
	temp_frame = 0;
}

OilMain::~OilMain()
{
	if(defaults) delete defaults;
	if(temp_frame) delete temp_frame;
}

char* OilMain::plugin_title() { return "Oil Painting"; }
int OilMain::plugin_is_realtime() { return 1; }
int OilMain::plugin_is_multi_channel() { return 0; }

int OilMain::start_realtime()
{
	redo_buffers = 1;
return 0;
}

int OilMain::stop_realtime()
{
return 0;
}

int OilMain::reconfigure()
{
	redo_buffers = 0;
return 0;
}

int OilMain::oil_rgb(VPixel **in_rows, VPixel **out_rows, int use_intensity)
{
	int x1, y1, x2, y2, x3, y3, x4, y4, n, c;
	VWORD Val[4];
	VWORD Cnt[4], Cnt2;
	VWORD Hist[4][HISTSIZE], Hist2[HISTSIZE];
	VPixel *src, *dest;

	n = radius / 2;

	for(y1 = 0; y1 < project_frame_h; y1++)
	{
		dest = out_rows[y1];

		if(!use_intensity)
		{
// Use RGB
			for(x1 = 0; x1 < project_frame_w; x1++)
			{
				memset(Cnt, 0, sizeof(Cnt));
				memset(Val, 0, sizeof(Val));
				memset(Hist, 0, sizeof(Hist));

				x3 = CLAMP((x1 - n), 0, project_frame_w);
	    		y3 = CLAMP((y1 - n), 0, project_frame_h);
	    		x4 = CLAMP((x1 + n + 1), 0, project_frame_w);
	    		y4 = CLAMP((y1 + n + 1), 0, project_frame_h);

				for(y2 = y3; y2 < y4; y2++)
				{
					src = in_rows[y2];
					for(x2 = x3; x2 < x4; x2++)
					{
						if((c = ++Hist[0][src[x2].r]) > Cnt[0])
						{
							Val[0] = src[x2].r;
							Cnt[0] = c;
						}
						if((c = ++Hist[1][src[x2].g]) > Cnt[1])
						{
							Val[1] = src[x2].g;
							Cnt[1] = c;
						}
						if((c = ++Hist[2][src[x2].b]) > Cnt[2])
						{
							Val[2] = src[x2].b;
							Cnt[2] = c;
						}
						if((c = ++Hist[3][src[x2].a]) > Cnt[3])
						{
							Val[3] = src[x2].a;
							Cnt[3] = c;
						}
					}
				}

				dest[x1].r = Val[0];
				dest[x1].g = Val[1];
				dest[x1].b = Val[2];
				dest[x1].a = Val[3];
			}
		}
		else
		{
// Use intensity
			for(x1 = 0; x1 < project_frame_w; x1++)
			{
				Cnt2 = 0;
				memset(Val, 0, sizeof(Val));
				memset(Hist2, 0, sizeof(Hist2));

				x3 = CLAMP((x1 - n), 0, project_frame_w);
	    		y3 = CLAMP((y1 - n), 0, project_frame_h);
	    		x4 = CLAMP((x1 + n + 1), 0, project_frame_w);
	    		y4 = CLAMP((y1 + n + 1), 0, project_frame_h);

				for(y2 = y3; y2 < y4; y2++)
				{
					src = in_rows[y2];
					for(x2 = x3; x2 < x4; x2++)
					{
						if((c = ++Hist2[INTENSITY(src[x2])]) > Cnt2)
						{
							Val[0] = src[x2].r;
							Val[1] = src[x2].g;
							Val[2] = src[x2].b;
							Val[3] = src[x2].a;
							Cnt2 = c;
						}
					}
				}

				dest[x1].r = Val[0];
				dest[x1].g = Val[1];
				dest[x1].b = Val[2];
				dest[x1].a = Val[3];
			}
		}
	}
return 0;
}

int OilMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i, j, k, l;
	VPixel **input_rows, **output_rows;

	if(redo_buffers) reconfigure();

	for(i = 0; i < size; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();
//printf("%d\n", input_ptr[i]->equals(output_ptr[i]));
		if(radius == 0)
		{
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
		else
		{
// Process oil painting
			if(buffers_identical(0))
			{
				if(!temp_frame) temp_frame = new VFrame(0, project_frame_w, project_frame_h);
				temp_frame->copy_from(input_ptr[i]);
				input_rows = (VPixel**)temp_frame->get_rows();
			}
			oil_rgb(input_rows, output_rows, use_intensity);
		}
	}
return 0;
}

int OilMain::start_gui()
{
	load_defaults();
	thread = new OilThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int OilMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int OilMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int OilMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int OilMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int OilMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%soil.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	radius = defaults->get("RADIUS", radius);
	use_intensity = defaults->get("USEINTENSITY", use_intensity);
return 0;
}

int OilMain::save_defaults()
{
	defaults->update("RADIUS", radius);
	defaults->update("USEINTENSITY", use_intensity);
	defaults->save();
return 0;
}

int OilMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("OIL");
	output.tag.set_property("RADIUS", radius);
	output.tag.set_property("USEINTENSITY", use_intensity);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int OilMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("OIL"))
			{
				radius = input.tag.get_property("RADIUS", radius);
				use_intensity = input.tag.get_property("USEINTENSITY", use_intensity);
			}
		}
	}
	if(thread) 
	{
		thread->window->radius->update(radius);
		thread->window->use_intensity->update(use_intensity);
	}
	redo_buffers = 1;
return 0;
}
