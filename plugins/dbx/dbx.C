#include "dbx.h"
#include "dbxwindow.h"
#include "defaults.h"
#include "filehtal.h"

#include <math.h>
#include <string.h>

int main(int argc, char *argv[])
{
	DBXMain *plugin;
	
	plugin = new DBXMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

DBXMain::DBXMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	thread = 0;
	gain = 0;
	window = 1024;
}

DBXMain::~DBXMain()
{
}

const char* DBXMain::plugin_title() { return "DBX"; }
int DBXMain::plugin_is_realtime() { return 1; }
int DBXMain::plugin_is_multi_channel() { return 0; }
	
int DBXMain::start_realtime()
{
	rms_total = 0;
	rms_size = 0;
	rms_position = 0;
	for(int i = 0; i < RMSLEN; i++)
		rms_data[i] = 0;
return 0;
}

int DBXMain::stop_realtime()
{
return 0;
}

int DBXMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sdbx.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();
	
	gain = defaults->get("GAIN", 0.0);
	window = defaults->get("WINDOW", 1024);
return 0;
}

int DBXMain::save_defaults()
{
	defaults->update("GAIN", gain);
	defaults->update("WINDOW", window);
	defaults->save();
return 0;
}

void DBXMain::store_rms_data(float value)
{
//	rms_total -= rms_data[rms_position] * rms_data[rms_position];
	rms_total -= rms_data[rms_position];
	rms_data[rms_position] = fabs(value);
//	rms_total += rms_data[rms_position] * rms_data[rms_position];
	rms_total += rms_data[rms_position];
	rms_position++;
	if(rms_position >= window) rms_position = 0;
	rms_size++;
	if(rms_size > window) rms_size = window;
}

int DBXMain::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	float level = DB::fromdb(gain);
	for( int j = 0; j < size; j++) 
	{
		store_rms_data(input_ptr[j]);
//		double rms_value = sqrt(rms_total / rms_size);
		double rms_value = rms_total / rms_size;

		output_ptr[j] *= DB::fromdb(DB::todb(rms_value) * 2.0) / rms_value;
		output_ptr[j] *= level;
	}
	return 0;
}


int DBXMain::start_gui()
{
	load_defaults();
	thread = new DBXThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int DBXMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int DBXMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int DBXMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int DBXMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int DBXMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("GAIN");
	output.tag.set_property("VALUE", gain);
	output.append_tag();
	output.tag.set_title("WINDOW");
	output.tag.set_property("VALUE", window);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int DBXMain::read_data(char *text)
{
	FileHTAL input;
	
	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("GAIN"))
			{
				gain = input.tag.get_property("VALUE", gain);
			}
			else
			if(input.tag.title_is("WINDOW"))
			{
				window = input.tag.get_property("VALUE", window);
			}
		}
	}
	if(thread)
	{
		thread->window->gain->update(gain);
		thread->window->window->update(window);
	}
	return 0;
}
