#include "filehtal.h"
#include "rgb601.h"
#include "rgb601window.h"

int main(int argc, char *argv[])
{
	RGB601Main *plugin;

	plugin = new RGB601Main(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

RGB601Main::RGB601Main(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	rgb_to_601 = 0;
	_601_to_rgb = 0;
}

RGB601Main::~RGB601Main()
{
}

char* RGB601Main::plugin_title() { return "RGB ... 601"; }
int RGB601Main::plugin_is_realtime() { return 1; }
int RGB601Main::plugin_is_multi_channel() { return 0; }
	
int RGB601Main::start_realtime()
{
	int _601_to_rgb_value;
	for(int i = 0; i <= VMAX; i++)
	{
		rgb_to_601_table[i] = (VWORD)(0.8588 * i + VMAX * 0.0627 + 0.5);
		_601_to_rgb_value = (int)(1.1644 * i - VMAX * 0.0627 + 0.5);
		if(_601_to_rgb_value < 0) _601_to_rgb_value = 0;
		if(_601_to_rgb_value > VMAX) _601_to_rgb_value = VMAX;
		_601_to_rgb_table[i] = _601_to_rgb_value;
	}
}

int RGB601Main::stop_realtime()
{
}

int RGB601Main::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k, l;
	VPixel **input_rows, **output_rows;
	VPixel *input_row, *output_row;

	for(i = 0; i < size; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();

// Copy to new frame if necessary
		if(!rgb_to_601 && !_601_to_rgb && !buffers_identical(i))
		{
			output_ptr[i]->copy_from(input_ptr[i]);
		}
		else
		{
			if(rgb_to_601)
			{
				for(i = 0; i < project_frame_h; i++)
				{
					input_row = input_rows[i];
					output_row = output_rows[i];
					for(k = 0; k < project_frame_w; k++)
					{
						output_row[k].r = rgb_to_601_table[input_row[k].r];
						output_row[k].g = rgb_to_601_table[input_row[k].g];
						output_row[k].b = rgb_to_601_table[input_row[k].b];
						output_row[k].a = input_row[k].a;
					}
				}
			}
			else
			if(_601_to_rgb)
			{
				for(i = 0; i < project_frame_h; i++)
				{
					input_row = input_rows[i];
					output_row = output_rows[i];
					for(k = 0; k < project_frame_w; k++)
					{
						output_row[k].r = _601_to_rgb_table[input_row[k].r];
						output_row[k].g = _601_to_rgb_table[input_row[k].g];
						output_row[k].b = _601_to_rgb_table[input_row[k].b];
						output_row[k].a = input_row[k].a;
					}
				}
			}
		}
	}
}

int RGB601Main::start_gui()
{
	load_defaults();
	thread = new RGB601Thread(this);
	thread->start();
	thread->gui_started.lock();
}

int RGB601Main::stop_gui()
{
	cleanup_gui();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int RGB601Main::show_gui()
{
	thread->window->show_window();
}

int RGB601Main::hide_gui()
{
	thread->window->hide_window();
}

int RGB601Main::cleanup_gui()
{
	save_defaults();
}


int RGB601Main::set_string()
{
	thread->window->set_title(gui_string);
}

int RGB601Main::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	if(rgb_to_601)
	{
		output.tag.set_title("RGB601");
		output.append_tag();
	}

	if(_601_to_rgb)
	{
		output.tag.set_title("601RGB");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
}

int RGB601Main::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	rgb_to_601 = _601_to_rgb = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("RGB601"))
			{
				rgb_to_601 = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
			else
			if(input.tag.title_is("601RGB"))
			{
				_601_to_rgb = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
		}
	}
	if(thread) 
	{
		thread->window->rgb_to_601->update(rgb_to_601);
		thread->window->_601_to_rgb->update(_601_to_rgb);
	}
}


int RGB601Main::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%srgb601.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	rgb_to_601 = defaults->get("RGB601", 0);
	_601_to_rgb = defaults->get("601RGB", 0);
}

int RGB601Main::save_defaults()
{
	defaults->update("RGB601", rgb_to_601);
	defaults->update("601RGB", _601_to_rgb);
	defaults->save();
}
