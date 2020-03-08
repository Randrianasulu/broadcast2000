#include "filehtal.h"
#include "bluescreen.h"
#include "bluewindow.h"

main(int argc, char *argv[])
{
	BluescreenMain *plugin;

	plugin = new BluescreenMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

BluescreenMain::BluescreenMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	h = 0;
	s = 0;
	v = 0;
	threshold = 0;
	feather = 0;
}

BluescreenMain::~BluescreenMain()
{
	if(defaults) delete defaults;
}

char* BluescreenMain::plugin_title() { return "Chroma key"; }
int BluescreenMain::plugin_is_realtime() { return 1; }
int BluescreenMain::plugin_is_multi_channel() { return 0; }
	
int BluescreenMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;

	engine = new  BluescreenEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new BluescreenEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
}

int BluescreenMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
}

int BluescreenMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;

	for(i = 0; i < smp; i++)
	{
		engine[i]->start_process_frame(output_ptr, input_ptr, size);
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_process_frame();
	}
}

int BluescreenMain::start_gui()
{
	if(!thread)
	{
		load_defaults();
		thread = new BlueThread(this);
		thread->start();
		thread->gui_started.lock();
	}
}

int BluescreenMain::stop_gui()
{
	if(thread)
	{
		save_defaults();
		thread->window->set_done(0);
		thread->join();
		delete thread;
		thread = 0;
	}
}

int BluescreenMain::show_gui()
{
	thread->window->show_window();
}

int BluescreenMain::hide_gui()
{
	thread->window->hide_window();
}

int BluescreenMain::set_string()
{
	thread->window->set_title(gui_string);
}

int BluescreenMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sbluescreen.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	h = defaults->get("HUE", (float)0);
	s = defaults->get("SATURATION", (float)0);
	v = defaults->get("VALUE", (float)0);
	threshold = defaults->get("THRESHOLD", (float)0);
	feather = defaults->get("FEATHER", (float)0);
}

int BluescreenMain::save_defaults()
{
	defaults->update("HUE", h);
	defaults->update("SATURATION", s);
	defaults->update("VALUE", v);
	defaults->update("THRESHOLD", threshold);
	defaults->update("FEATHER", feather);
	defaults->save();
}

int BluescreenMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("HUE");
	output.tag.set_property("VALUE", h);
	output.append_tag();
	output.tag.set_title("SATURATION");
	output.tag.set_property("VALUE", s);
	output.append_tag();
	output.tag.set_title("VALUE");
	output.tag.set_property("VALUE", v);
	output.append_tag();
	output.tag.set_title("RED");
	output.tag.set_property("VALUE", r);
	output.append_tag();
	output.tag.set_title("GREEN");
	output.tag.set_property("VALUE", g);
	output.append_tag();
	output.tag.set_title("BLUE");
	output.tag.set_property("VALUE", b);
	output.append_tag();
	output.tag.set_title("THRESHOLD");
	output.tag.set_property("VALUE", threshold);
	output.append_tag();
	output.tag.set_title("FEATHER");
	output.tag.set_property("VALUE", feather);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int BluescreenMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("HUE"))
			{
				h = input.tag.get_property("VALUE", h);
			}
			else
			if(input.tag.title_is("SATURATION"))
			{
				s = input.tag.get_property("VALUE", s);
			}
			else
			if(input.tag.title_is("VALUE"))
			{
				v = input.tag.get_property("VALUE", v);
			}
			else
			if(input.tag.title_is("RED"))
			{
				r = input.tag.get_property("VALUE", r);
			}
			else
			if(input.tag.title_is("GREEN"))
			{
				g = input.tag.get_property("VALUE", g);
			}
			else
			if(input.tag.title_is("BLUE"))
			{
				b = input.tag.get_property("VALUE", b);
			}
			else
			if(input.tag.title_is("THRESHOLD"))
			{
				threshold = input.tag.get_property("VALUE", threshold);
			}
			else
			if(input.tag.title_is("FEATHER"))
			{
				feather = input.tag.get_property("VALUE", feather);
			}
		}
	}

	if(thread) 
	{
// update the GUI here
		update_display();
	}
}

int BluescreenMain::update_rgb()
{
	if(thread)
	{
		rgb_to_hsv(thread->window->red->get_value(), 
					thread->window->green->get_value(), 
					thread->window->blue->get_value(), 
					h, s, v);
		update_display();
	}
}

int BluescreenMain::update_display()
{
	if(thread)
	{
		float r, g, b;
		if(h < 0) h = 0;
		if(h > 360) h = 360;
		if(s < 0) s = 0;
		if(s > 1) s = 1;
		if(v < 0) v = 0;
		if(v > 1) v = 1;

		thread->window->wheel->draw(thread->window->wheel->oldhue, 
									thread->window->wheel->oldsaturation);
		thread->window->wheel->oldhue = h;
		thread->window->wheel->oldsaturation = s;
		thread->window->wheel->draw(h, s);
		thread->window->wheel->flash();
		thread->window->wheel_value->draw(h, s, v);
		thread->window->wheel_value->flash();
		thread->window->output->draw();
		thread->window->output->flash();
		thread->window->hue->update(h);
		thread->window->saturation->update(s);
		thread->window->value->update(v);
		
		hsv_to_rgb(r, g, b, h, s, v);
		thread->window->red->update(r);
		thread->window->green->update(g);
		thread->window->blue->update(b);
	}
}

int BluescreenMain::rgb_to_hsv(float r, float g, float b, float &h, float &s, float &v)
{
	float min, max, delta;

	min = ((r < g) ? r : g) < b ? ((r < g) ? r : g) : b;
	max = ((r > g) ? r : g) > b ? ((r > g) ? r : g) : b;
	v = max;                               // v

	delta = max - min;

	if(max != 0)
        s = delta / max;               // s
	else 
	{
        // r = g = b = 0                // s = 0, v is undefined
        s = 0;
        h = -1;
        return 0;
	}

	if(r == max)
        h = (g - b) / delta;         // between yellow & magenta
	else 
	if(g == max)
        h = 2 + (b - r) / delta;     // between cyan & yellow
	else
        h = 4 + (r - g) / delta;     // between magenta & cyan

	h *= 60;                               // degrees
	if(h < 0)
        h += 360;
}

int BluescreenMain::hsv_to_rgb(float &r, float &g, float &b, float h, float s, float v)
{
    int i;
    float f, p, q, t;

    if(s == 0) 
	{
        // achromatic (grey)
        r = g = b = v;
        return 0;
    }

    h /= 60;                        // sector 0 to 5
    i = (int)h;
    f = h - i;                      // factorial part of h
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch(i) 
	{
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:                // case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
}

BluescreenEngine::BluescreenEngine(BluescreenMain *plugin, int start_y, int end_y)
 : Thread()
{
	this->plugin = plugin;
	this->start_y = start_y;
	this->end_y = end_y;
	last_frame = 0;
	input_lock.lock();
	output_lock.lock();
}

BluescreenEngine::~BluescreenEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int BluescreenEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
}

int BluescreenEngine::wait_process_frame()
{
	output_lock.lock();
}

void BluescreenEngine::run()
{
	int i, j, k;
	int r, g, b;
	float r_f, g_f, b_f;
	int min_r, max_r;
	int min_g, max_g;
	int min_b, max_b;
	float threshold2;
	VPixel **in_rows, **out_rows;

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		hsv.hsv_to_rgb(r_f, g_f, b_f, plugin->h, plugin->s, plugin->v);
		r = (int)(r_f * VMAX);
		g = (int)(g_f * VMAX);
		b = (int)(b_f * VMAX);

		threshold2 = plugin->threshold / 2;

		min_r = (int)(r - threshold2);
		min_g = (int)(g - threshold2);
		min_b = (int)(b - threshold2);
		max_r = (int)(r + threshold2 + .5);
		max_g = (int)(g + threshold2 + .5);
		max_b = (int)(b + threshold2 + .5);

		if(min_r < 0) min_r = 0;
		if(min_g < 0) min_g = 0;
		if(min_b < 0) min_b = 0;
		if(max_r > VMAX) max_r = VMAX;
		if(max_g > VMAX) max_g = VMAX;
		if(max_b > VMAX) max_b = VMAX;

		for(i = 0; i < size; i++)
		{
			in_rows = ((VPixel**)input[i]->get_rows());
			out_rows = ((VPixel**)output[i]->get_rows());

			for(j = 0; j < plugin->project_frame_h; j++)
			{
				for(k = 0; k < plugin->project_frame_w; k++)
				{
					r = in_rows[j][k].r;
					g = in_rows[j][k].g;
					b = in_rows[j][k].b;

					if(
						r >= min_r && r <= max_r && 
						g >= min_g && g <= max_g && 
						b >= min_b && b <= max_b
					)
					{
// Convert alpha to transparent
						out_rows[j][k].a = 0;
					}
					else
					{
// Keep alpha
						out_rows[j][k].a = in_rows[j][k].a;
					}
					out_rows[j][k].r = r;
					out_rows[j][k].g = g;
					out_rows[j][k].b = b;
				}
			}
		}
		output_lock.unlock();
	}
}
