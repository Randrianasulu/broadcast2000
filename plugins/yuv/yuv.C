#include "filehtal.h"
#include "yuv.h"
#include "yuvwindow.h"

int main(int argc, char *argv[])
{
	YUVMain *plugin;

	plugin = new YUVMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

YUVMain::YUVMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	automated_function = 0;
	y = 0;
	u = 0;
	v = 0;
	defaults = 0;
	reconfigure_flag = 0;
}

YUVMain::~YUVMain()
{
	if(defaults) delete defaults;
}

char* YUVMain::plugin_title() { return "YUV"; }
int YUVMain::plugin_is_realtime() { return 1; }
int YUVMain::plugin_is_multi_channel() { return 0; }
	
int YUVMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;
	reconfigure_flag = 1;

	engine = new YUVEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new YUVEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
return 0;
}

int YUVMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
return 0;
}


int YUVMain::start_gui()
{
	load_defaults();
	thread = new YUVThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int YUVMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int YUVMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int YUVMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int YUVMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int YUVMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, BCASTDIR "yuv.rc");

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	y = defaults->get("Y", 0);
	u = defaults->get("U", 0);
	v = defaults->get("V", 0);
	automated_function = defaults->get("AUTOMATION", automated_function);
return 0;
}

int YUVMain::save_defaults()
{
	defaults->update("Y", y);
	defaults->update("U", u);
	defaults->update("V", v);
	defaults->update("AUTOMATION", automated_function);
	defaults->save();
return 0;
}

int YUVMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("Y");
	output.tag.set_property("VALUE", y);
	output.append_tag();
	output.tag.set_title("U");
	output.tag.set_property("VALUE", u);
	output.append_tag();
	output.tag.set_title("V");
	output.tag.set_property("VALUE", v);
	output.append_tag();
	output.tag.set_title("AUTOMATED");
	output.tag.set_property("FUNCTION", automated_function);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int YUVMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("Y"))
			{
				y = input.tag.get_property("VALUE", y);
			}
			else
			if(input.tag.title_is("U"))
			{
				u = input.tag.get_property("VALUE", u);
			}
			else
			if(input.tag.title_is("V"))
			{
				v = input.tag.get_property("VALUE", v);
			}
			else
			if(input.tag.title_is("AUTOMATED"))
			{
				automated_function = input.tag.get_property("FUNCTION", automated_function);
			}
		}
	}
	if(thread)
	{
		thread->window->y_slider->update(y);
		thread->window->u_slider->update(u);
		thread->window->v_slider->update(v);
		for(int i = 0; i < 3; i++)
		{
			thread->window->automation[i]->update(automated_function == i);
		}
	}
return 0;
}

int YUVMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;
	int old_y = y;
	int old_u = u;
	int old_v = v;

	if(automation_used())
	{
		switch(automated_function)
		{
			case 0:
				y += (int)(get_automation_value(0) * MAXVALUE);
				break;
			case 1:
				u += (int)(get_automation_value(0) * MAXVALUE);
				break;
			case 2:
				v += (int)(get_automation_value(0) * MAXVALUE);
				break;
		}
		if(y > MAXVALUE) y = MAXVALUE;
		if(y < -MAXVALUE) y = -MAXVALUE;
		if(u > MAXVALUE) u = MAXVALUE;
		if(u < -MAXVALUE) u = -MAXVALUE;
		if(v > MAXVALUE) v = MAXVALUE;
		if(v < -MAXVALUE) v = -MAXVALUE;
	}

	if(old_y != y || old_u != u || old_v != v || reconfigure_flag)
		reconfigure();

	for(i = 0; i < smp; i++)
	{
		engine[i]->start_process_frame(output_ptr, input_ptr, size);
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_process_frame();
	}

	y = old_y;
	u = old_u;
	v = old_v;
return 0;
}

int YUVMain::reconfigure()
{
return 0;
}

YUVEngine::YUVEngine(YUVMain *plugin, int start_y, int end_y)
 : Thread()
{
	this->plugin = plugin;
	this->start_y = start_y;
	this->end_y = end_y;
	last_frame = 0;
	input_lock.lock();
	output_lock.lock();
}

YUVEngine::~YUVEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int YUVEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
return 0;
}

int YUVEngine::wait_process_frame()
{
	output_lock.lock();
return 0;
}

void YUVEngine::run()
{
	int i, j, k;
	VPixel **input_rows, **output_rows;
	long y, u, v, r, g, b;
	long y_scale, u_scale, v_scale;

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		y_scale = (long)((float)(plugin->y + MAXVALUE) / MAXVALUE * 256);
		u_scale = (long)((float)(plugin->u + MAXVALUE) / MAXVALUE * 256);
		v_scale = (long)((float)(plugin->v + MAXVALUE) / MAXVALUE * 256);

		for(i = 0; i < size; i++)
		{
			input_rows = (VPixel**)input[i]->get_rows();
			output_rows = (VPixel**)output[i]->get_rows();

			for(j = start_y; j < end_y; j++)
			{
				for(k = 0; k < plugin->project_frame_w; k++)
				{
					r = input_rows[j][k].r;
					g = input_rows[j][k].g;
					b = input_rows[j][k].b;
					yuv.rgb_to_yuv(r, g, b, y, u, v);
					y *= y_scale;
					u *= u_scale;
					v *= v_scale;
					y >>= 8;
					u >>= 8;
					v >>= 8;
					if(y > VMAX) y = VMAX;
					else
					if(y < 0) y = 0;
					if(u > VMAX / 2) u = VMAX / 2;
					else
					if(u < -VMAX / 2) u = -VMAX / 2;
					if(v > VMAX / 2) v = VMAX / 2;
					else
					if(v < -VMAX / 2) v = -VMAX / 2;

					yuv.yuv_to_rgb(r, g, b, y, u, v);

					output_rows[j][k].r = r;
					output_rows[j][k].g = g;
					output_rows[j][k].b = b;
					output_rows[j][k].a = input_rows[j][k].a;
				}
			}
		}

		output_lock.unlock();
	}
}
