#include <string.h>

#include "defaults.h"
#include "filehtal.h"
#include "freeverb.h"
#include "freeverbwin.h"

int main(int argc, char *argv[])
{
	Freeverb *plugin;

	plugin = new Freeverb(argc, argv);
	plugin->plugin_run();
	delete plugin;
	return 0;
}

Freeverb::Freeverb(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	defaults = 0;
	redo_buffers = 1;       // set to redo buffers before the first render
	thread = 0;
}

Freeverb::~Freeverb()
{
// never called
	if(defaults) delete defaults;
}

int Freeverb::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%sreverb.rc", get_defaultdir());
	
// load the defaults

	defaults = new Defaults(directory);
	defaults->load();

	roomsize = defaults->get("ROOM_SIZE", 0.5f);
	damping = defaults->get("DAMPING", 0.5f);
	wetness = defaults->get("WETNESS", 0.333f);
	dryness = defaults->get("DRYNESS", 0);
	gain = defaults->get("GAIN", 0.0f);
	return 0;
}

int Freeverb::save_defaults()
{
	defaults->update("ROOM_SIZE", roomsize);
	defaults->update("DAMPING", damping);
	defaults->update("WETNESS", wetness);
	defaults->update("DRYNESS", dryness);
	defaults->update("GAIN", gain);
	defaults->save();
	return 0;
}


const char* Freeverb::plugin_title() { return "Freeverb"; }
int Freeverb::plugin_is_realtime() { return 1; }
int Freeverb::plugin_is_multi_channel() { return 1; }

int Freeverb::start_realtime()
{
	revmodel::combtuningL1 *= project_sample_rate;
	revmodel::combtuningR1 *= project_sample_rate;
	revmodel::combtuningL2 *= project_sample_rate;
	revmodel::combtuningR2 *= project_sample_rate;
	revmodel::combtuningL3 *= project_sample_rate;
	revmodel::combtuningR3 *= project_sample_rate;
	revmodel::combtuningL4 *= project_sample_rate;
	revmodel::combtuningR4 *= project_sample_rate;
	revmodel::combtuningL5 *= project_sample_rate;
	revmodel::combtuningR5 *= project_sample_rate;
	revmodel::combtuningL6 *= project_sample_rate;
	revmodel::combtuningR6 *= project_sample_rate;
	revmodel::combtuningL7 *= project_sample_rate;
	revmodel::combtuningR7 *= project_sample_rate;
	revmodel::combtuningL8 *= project_sample_rate;
	revmodel::combtuningR8 *= project_sample_rate;
	revmodel::allpasstuningL1 *= project_sample_rate;
	revmodel::allpasstuningR1 *= project_sample_rate;
	revmodel::allpasstuningL2 *= project_sample_rate;
	revmodel::allpasstuningR2 *= project_sample_rate;
	revmodel::allpasstuningL3 *= project_sample_rate;
	revmodel::allpasstuningR3 *= project_sample_rate;
	revmodel::allpasstuningL4 *= project_sample_rate;
	revmodel::allpasstuningR4 *= project_sample_rate;

	revmodel::combtuningL1 /= 44100;
	revmodel::combtuningR1 /= 44100;
	revmodel::combtuningL2 /= 44100;
	revmodel::combtuningR2 /= 44100;
	revmodel::combtuningL3 /= 44100;
	revmodel::combtuningR3 /= 44100;
	revmodel::combtuningL4 /= 44100;
	revmodel::combtuningR4 /= 44100;
	revmodel::combtuningL5 /= 44100;
	revmodel::combtuningR5 /= 44100;
	revmodel::combtuningL6 /= 44100;
	revmodel::combtuningR6 /= 44100;
	revmodel::combtuningL7 /= 44100;
	revmodel::combtuningR7 /= 44100;
	revmodel::combtuningL8 /= 44100;
	revmodel::combtuningR8 /= 44100;
	revmodel::allpasstuningL1 /= 44100;
	revmodel::allpasstuningR1 /= 44100;
	revmodel::allpasstuningL2 /= 44100;
	revmodel::allpasstuningR2 /= 44100;
	revmodel::allpasstuningL3 /= 44100;
	revmodel::allpasstuningR3 /= 44100;
	revmodel::allpasstuningL4 /= 44100;
	revmodel::allpasstuningR4 /= 44100;


	engine = new revmodel();
	
	tmp_output = new float*[total_in_buffers];
	for(int i = 0; i < total_in_buffers; i++)
	{
		tmp_output[i] = new float[1];
	}
	tmp_size = 1;
	redo_buffers = 1;
	return 0;
}

int Freeverb::stop_realtime()
{
	delete engine;
	for(int i = 0; i < total_in_buffers; i++)
	{
		delete [] tmp_output[i];
	}
	delete [] tmp_output;
	return 0;
}

int Freeverb::process_realtime(long size, float **input_ptr, float **output_ptr)
{
	if(redo_buffers)
	{
		engine->setroomsize((float)roomsize / 100);
		engine->setdamp(db.fromdb(damping));
		engine->setwet(db.fromdb(wetness));
		engine->setdry(db.fromdb(dryness));
		redo_buffers = 0;
	}

	if(tmp_size != size)
	{
		for(int i = 0; i < total_in_buffers; i++)
		{
			delete [] tmp_output[i];
			tmp_output[i] = new float[size];
		}
		tmp_size = size;
	}

	if(total_in_buffers < 2)
	{
		engine->processreplace(input_ptr[0], 
			input_ptr[0], 
			tmp_output[0], 
			tmp_output[0], size, 1);
	}
	else
	{
		engine->processreplace(input_ptr[0], 
			input_ptr[1], 
			tmp_output[0], 
			tmp_output[1], size, 1);
	}

	float gain_f = db.fromdb(gain);
	for(int i = 0; i < total_in_buffers; i++)
	{
		for(int j = 0; j < size; j++)
			output_ptr[i][j] = tmp_output[i][j] * gain_f;
	}
	return 0;
}



int Freeverb::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new FreeverbThread(this);
	thread->set_synchronous(1);
	thread->start();
	thread->gui_started.lock();
	return 0;
}

int Freeverb::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
	save_defaults();
	return 0;
}

int Freeverb::show_gui()
{
	thread->window->show_window();
	return 0;
}

int Freeverb::hide_gui()
{
	thread->window->hide_window();
	return 0;
}

int Freeverb::set_string()
{
	thread->window->set_title(gui_string);
	return 0;
}

int Freeverb::save_data(char *text)
{
	FileHTAL output;

// cause htal file to store data directly in text
	output.set_shared_string(text, MESSAGESIZE);
	
	output.tag.set_title("FREEVERB");
	output.tag.set_property("ROOM_SIZE", roomsize);
	output.tag.set_property("DAMPING", damping);
	output.tag.set_property("WETNESS", wetness);
	output.tag.set_property("DRYNESS", dryness);
	output.tag.set_property("GAIN", gain);
	output.append_tag();
	output.append_newline();

	output.terminate_string();
// data is now in *text
	return 0;
}

int Freeverb::read_data(char *text)
{
	FileHTAL input;
// cause htal file to read directly from text
	input.set_shared_string(text, strlen(text));
	int result = 0;

	result = input.read_tag();

	if(!result)
	{
		if(input.tag.title_is("FREEVERB"))
		{
			roomsize = input.tag.get_property("ROOM_SIZE", roomsize);
			damping = input.tag.get_property("DAMPING", damping);
			wetness = input.tag.get_property("WETNESS", wetness);
			dryness = input.tag.get_property("DRYNESS", dryness);
			gain = input.tag.get_property("GAIN", gain);
			redo_buffers = 1;
			update_gui();
		}
	}
	return 0;
}

int Freeverb::update_gui()
{
	if(thread)
	{
		thread->window->roomsize->update(roomsize);
		thread->window->damping->update(damping);
		thread->window->wetness->update(wetness);
		thread->window->dryness->update(dryness);
		thread->window->gain->update(gain);
	}
	return 0;
}
