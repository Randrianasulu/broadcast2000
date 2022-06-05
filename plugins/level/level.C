#include "filehtal.h"
#include "level.h"
#include "levelwindow.h"

int main(int argc, char *argv[])
{
	LevelMain *plugin;
	
	plugin = new LevelMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

LevelMain::LevelMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	level = 0;
	thread = 0;
}

LevelMain::~LevelMain()
{
}

const char* LevelMain::plugin_title() { return "Gain"; }
int LevelMain::plugin_is_realtime() { return 1; }
int LevelMain::plugin_is_multi_channel() { return 0; }
	
int LevelMain::start_realtime()
{
return 0;
}

int LevelMain::stop_realtime()
{
return 0;
}

int LevelMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%slevel.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	level = defaults->get("LEVEL", (float)0);
return 0;
}

int LevelMain::save_defaults()
{
	defaults->update("LEVEL", level);
	defaults->save();
return 0;
}
	
int LevelMain::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	float new_level = level;

	if(automation_used())
	{
		 float scale;
		for( int j = 0; j < size; j++)
		{
			scale = db.fromdb(level + get_automation_value(j) * MAXLEVEL);
			output_ptr[j] = input_ptr[j] * scale;
		}
	}
	else
	{
		float scale = db.fromdb(new_level);
		for( int j = 0; j < size; j++) output_ptr[j] = input_ptr[j] * scale;
	}
return 0;
}


int LevelMain::start_gui()
{
	load_defaults();
	thread = new LevelThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int LevelMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int LevelMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int LevelMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int LevelMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int LevelMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("LEVEL");
	output.tag.set_property("VALUE", (float)level);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int LevelMain::read_data(char *text)
{
	FileHTAL input;
	
	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("LEVEL"))
			{
				level = input.tag.get_property("VALUE", (float)level);
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
		}
	}
	if(thread) thread->window->slider->update(level);
return 0;
}
