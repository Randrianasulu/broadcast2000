#include "filehtal.h"
#include "huesaturation.h"
#include "huewindow.h"

int main(int argc, char *argv[])
{
	HueMain *plugin;

	plugin = new HueMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

HueMain::HueMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	automated_function = 0;
	hue = 0;
	saturation = 0;
	value = 0;
	defaults = 0;
	reconfigure_flag = 0;
}

HueMain::~HueMain()
{
	if(defaults) delete defaults;
}

char* HueMain::plugin_title() { return "Hue-Saturation"; }
int HueMain::plugin_is_realtime() { return 1; }
int HueMain::plugin_is_multi_channel() { return 0; }
	
int HueMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;
	reconfigure_flag = 1;

	engine = new HueEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new HueEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
}

int HueMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
}


int HueMain::start_gui()
{
	load_defaults();
	thread = new HueThread(this);
	thread->start();
	thread->gui_started.lock();
}

int HueMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int HueMain::show_gui()
{
	thread->window->show_window();
}

int HueMain::hide_gui()
{
	thread->window->hide_window();
}

int HueMain::set_string()
{
	thread->window->set_title(gui_string);
}

int HueMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%shuesaturation.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	hue = defaults->get("HUE", 0);
	saturation = defaults->get("SATURATION", 0);
	value = defaults->get("VALUE", 0);
	automated_function = defaults->get("AUTOMATION", automated_function);
}

int HueMain::save_defaults()
{
	defaults->update("HUE", hue);
	defaults->update("SATURATION", saturation);
	defaults->update("VALUE", value);
	defaults->update("AUTOMATION", automated_function);
	defaults->save();
}

int HueMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("HUE");
	output.tag.set_property("VALUE", hue);
	output.append_tag();
	output.tag.set_title("SATURATION");
	output.tag.set_property("VALUE", saturation);
	output.append_tag();
	output.tag.set_title("VALUE");
	output.tag.set_property("VALUE", value);
	output.append_tag();
	output.tag.set_title("AUTOMATED");
	output.tag.set_property("FUNCTION", automated_function);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int HueMain::read_data(char *text)
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
				hue = input.tag.get_property("VALUE", hue);
			}
			else
			if(input.tag.title_is("SATURATION"))
			{
				saturation = input.tag.get_property("VALUE", saturation);
			}
			else
			if(input.tag.title_is("VALUE"))
			{
				value = input.tag.get_property("VALUE", value);
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
		thread->window->hue_slider->update(hue);
		thread->window->saturation_slider->update(saturation);
		thread->window->value_slider->update(value);
		for(int i = 0; i < 3; i++)
		{
			thread->window->automation[i]->update(automated_function == i);
		}
	}
}

int HueMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;
	int old_hue = hue;
	int old_saturation = saturation;
	int old_value = value;

	if(automation_used())
	{
		switch(automated_function)
		{
			case 0:
				hue += (int)(get_automation_value(0) * MAXHUE);
				break;
			case 1:
				saturation += (int)(get_automation_value(0) * MAXVALUE);
				break;
			case 2:
				value += (int)(get_automation_value(0) * MAXVALUE);
				break;
		}
		if(hue > MAXHUE) hue = MAXHUE;
		if(hue < -MAXHUE) hue = -MAXHUE;
		if(saturation > MAXVALUE) saturation = MAXVALUE;
		if(saturation < -MAXVALUE) saturation = -MAXVALUE;
		if(value > MAXVALUE) value = MAXVALUE;
		if(value < -MAXVALUE) value = -MAXVALUE;
	}
	
	if(old_hue != hue || old_saturation != saturation || old_value != value || reconfigure_flag)
		reconfigure();

	for(i = 0; i < smp; i++)
	{
		engine[i]->start_process_frame(output_ptr, input_ptr, size);
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_process_frame();
	}

	hue = old_hue;
	saturation = old_saturation;
	value = old_value;
}

int HueMain::reconfigure()
{
}

HueEngine::HueEngine(HueMain *plugin, int start_y, int end_y)
 : Thread()
{
	this->plugin = plugin;
	this->start_y = start_y;
	this->end_y = end_y;
	last_frame = 0;
	input_lock.lock();
	output_lock.lock();
}

HueEngine::~HueEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int HueEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
}

int HueEngine::wait_process_frame()
{
	output_lock.lock();
}

void HueEngine::run()
{
	register int i, j, k;
	VPixel **input_rows, **output_rows;
	register float r, g, b, h, s, v;
	register int r_i, g_i, b_i;
	register float h_offset, s_offset, v_offset;

//printf("HueEngine::run 1\n");
	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		h_offset = plugin->hue;
		s_offset = (((float)plugin->saturation + 100)/ 100);
		v_offset = (((float)plugin->value + 100) / 100);
		for(i = 0; i < size; i++)
		{
			input_rows = ((VPixel**)input[i]->get_rows());
			output_rows = ((VPixel**)output[i]->get_rows());
#pragma omp parallel for schedule(static) private(j,k,r,g,b,h,s,v,r_i,g_i,b_i) num_threads(4) collapse(2)
			for(j = start_y; j < end_y; j++)
			{
				for(k = 0; k < plugin->project_frame_w; k++)
				{
					r = (float)input_rows[j][k].r / VMAX;
					g = (float)input_rows[j][k].g / VMAX;
					b = (float)input_rows[j][k].b / VMAX;
//printf("HueEngine::run 2\n");
					hsv.rgb_to_hsv(r, g, b, h, s, v);
//printf("HueEngine::run 3\n");

					h += h_offset;
					s *= s_offset;
					v *= v_offset;
					if(h >= 360) h -= 360;
					if(s > 1) s = 1;
					if(v > 1) v = 1;
					if(h < 0) h += 360;
					if(s < 0) s = 0;
					if(v < 0) v = 0;

//printf("HueEngine::run 4\n");
					hsv.hsv_to_rgb(r, g, b, h, s, v);
//printf("HueEngine::run 5\n");
					r_i = (int)(r * VMAX);
					g_i = (int)(g * VMAX);
					b_i = (int)(b * VMAX);
					if(r_i > VMAX) r_i = VMAX;
					if(g_i > VMAX) g_i = VMAX;
					if(b_i > VMAX) b_i = VMAX;
					if(r_i < 0) r_i = 0;
					if(g_i < 0) g_i = 0;
					if(b_i < 0) b_i = 0;
					output_rows[j][k].r = r_i;
					output_rows[j][k].g = g_i;
					output_rows[j][k].b = b_i;
					output_rows[j][k].a = input_rows[j][k].a;
				}
			}
		}

		output_lock.unlock();
	}
//printf("HueEngine::run 2\n");
}
