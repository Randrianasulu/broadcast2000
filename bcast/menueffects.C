#include <string.h>
#include "assets.h"
#include "defaults.h"
#include "errorbox.h"
#include "file.h"
#include "formatcheck.h"
#include "mainmenu.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "menueffects.h"
#include "neworappend.h"
#include "playbackengine.h"
#include "pluginarray.h"
#include "pluginserver.h"
#include "preferences.h"

MenuEffects::MenuEffects(MainWindow *mwindow)
 : BC_MenuItem("Effects...")
{
	this->mwindow = mwindow;
}

MenuEffects::~MenuEffects()
{
}


int MenuEffects::handle_event()
{
	thread->set_title("");
	thread->start();
return 0;
}







MenuEffectThread::MenuEffectThread(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	sprintf(title, "");
}

MenuEffectThread::~MenuEffectThread()
{
}


int MenuEffectThread::set_title(char *title)
{
	strcpy(this->title, title);
return 0;
}

// for recent effect menu items and running new effects
// prompts for an effect if title is blank
void MenuEffectThread::run()
{
// get stuff from main window
	ArrayList<PluginServer*> *plugindb = mwindow->plugindb;
	ArrayList<BC_ListBoxItem*> plugin_list;
	Defaults *defaults = mwindow->defaults;
	File file;
	char string[1024];
	int i;
	int result = 0;

//	sprintf(string, "");
//	defaults->get("EFFECTPATH", string);
	Asset asset;

// check for recordable tracks
	if(!get_recordable_tracks(&asset))
	{
		sprintf(string, "No recorable tracks specified.");
		ErrorBox error;
		error.create_objects(string);
		error.run_window();
		return;
	}

// check for plugins
	if(!plugindb->total)
	{
		sprintf(string, "No plugins available.");
		ErrorBox error;
		error.create_objects(string);
		error.run_window();
		return;
	}

// get selection to render
	long start, end;   // Range in native units
	long startsample, endsample;    // Range in samples for pasting
	
	mwindow->get_affected_range(&startsample, &endsample);
	start = startsample;
	end = endsample;
// get native units for the menu effect
	convert_units(start, end);

// Units are now in the track's units.
	long total_length = end - start;
	long output_start, output_end;        // length of output file

// get default attributes for output file
// used after completion
	get_derived_attributes(&asset, defaults);
	to_tracks = defaults->get("RENDERTOTRACKS", 1);

// get plugin information
	int need_plugin;
	if(!strlen(title)) need_plugin = 1; else need_plugin = 0;

// generate a list of plugins for the window
	if(need_plugin)
	{
		for(int i = 0; i < plugindb->total; i++)
		{
			if(!plugindb->values[i]->fileio && 
				((asset.audio_data && plugindb->values[i]->audio) ||
				(asset.video_data && plugindb->values[i]->video)))
			{
				plugin_list.append(new BC_ListBoxItem(plugindb->values[i]->title, BLACK));
			}
		}
	}

// find out which effect to run and get output file
	int plugin_number, format_error = 0;

	do
	{
		{
			MenuEffectWindow window(this, need_plugin ? &plugin_list : 0, &asset);
			window.create_objects();
			result = window.run_window();
			plugin_number = window.result;
		}

		if(!result)
		{
			FormatCheck format_check(&asset);
			format_error = format_check.check_format();
		}
	}while(format_error && !result);

// save defaults
//	defaults->update("EFFECTPATH", asset.path);
//	defaults->update("RENDERFORMAT", file.formattostr(mwindow->plugindb, asset.format));

	save_derived_attributes(&asset, defaults);
	defaults->update("RENDERTOTRACKS", to_tracks);


// get plugin server to use and delete the plugin list
	PluginServer *plugin = 0;
	if(need_plugin)
	{
		for(int i = 0; i < plugin_list.total; i++) delete plugin_list.values[i];
		plugin_list.remove_all();

// Weed out the right plugin based on its data type
		if(plugin_number > -1)
			for(int i = 0, total = 0; i < plugindb->total && !plugin; i++)
			{
				if(((asset.audio_data && plugindb->values[i]->audio) ||
					(asset.video_data && plugindb->values[i]->video)) &&
					(!plugindb->values[i]->fileio))
				{
					if(total == plugin_number)
					{
// Found it.
						plugin = plugindb->values[i];
						strcpy(title, plugindb->values[i]->title);
					}
					total++;
				}
			}
	}
	else
	{
		for(int i = 0; i < plugindb->total && !plugin; i++)
			if(!strcmp(plugindb->values[i]->title, title))
			{
				plugin = plugindb->values[i];
				plugin_number = i;
			}
	}

// Update the  most recently used effects.
	if(plugin)
	{
		fix_menu(title);
	}

	if(!result && !strlen(asset.path))
	{
		result = 1;        // no output path given
		sprintf(string, "No output file specified.");
		ErrorBox error;
		error.create_objects(string);
		error.run_window();
	}

// test existance of file	
	int append_to_file = 0;

	if(!result)
	{
		{
			NewOrAppend window;
// Append is not compatible with resample plugin.
			result = window.test_file("", &asset);
			switch(result)
			{
				case 0:
					mwindow->purge_asset(asset.path);
					remove(asset.path);              // overwrite
					append_to_file = 0;
					break;

				case 1:
					result = 1;                      // cancel
					break;

				case 2:
					append_to_file = 1;              // append
					result = 0;       // reset result;
					break;
			}
		}
	}

	char plugin_data[MESSAGESIZE];        // Message buffer for realtime plugins.

// ========================= get information from user
	if(!result)
	{
// ========================= realtime plugin 
// no get_parameters
		if(plugin->realtime)
		{
// Open a prompt GUI
			MenuEffectPrompt prompt;
			prompt.create_objects();
			char title[1024];
			sprintf(title, ICONNAME ": %s", plugin->title);

// Open the plugin GUI
			plugin->set_mainwindow(mwindow);
			plugin->open_plugin(0, &prompt, title);
			plugin->load_defaults();
//			plugin->start_gui(0, &prompt, title);

// wait for user input
			result = prompt.run_window();

// Close plugin
			plugin->save_defaults();

			if(!result)
			{
				strcpy(plugin_data, plugin->save_data());
			}
//			plugin->stop_gui();
			plugin->close_plugin();
			asset.rate = mwindow->sample_rate;
			asset.frame_rate = mwindow->frame_rate;
			realtime = 1;
		}
		else
// ============================non realtime plugin 
		{
			plugin->set_mainwindow(mwindow);
			plugin->open_plugin();
			plugin->load_defaults();
			result = plugin->get_parameters();
// some plugins can change the sample rate and the frame rate
			if(!result)
			{
				asset.rate = plugin->get_samplerate();
				asset.frame_rate = plugin->get_framerate();
			}

			plugin->save_defaults();
			plugin->close_plugin();
			realtime = 0;
		}
	}

// Open the output file after getting the information because the sample rate
// is needed here.
	if(!result)
	{
// open output file in read/write mode
		file.set_processors(mwindow->preferences->smp > 2 ? (mwindow->preferences->smp - 1) : 1);
		if(file.try_to_open_file(mwindow->plugindb, &asset, 1, 1))
		{
// open failed
			sprintf(string, "Couldn't open %s", asset.path);
			ErrorBox error;
			error.create_objects(string);
			error.run_window();
			result = 1;
		}
		else
		{
// dithering is done at the file for rendering
			if(dither) file.set_dither();
		}
	}

// run plugins
	if(!result)
	{
// position file

		if(!result)
		{
			if(asset.audio_data) output_start = file.get_audio_length();
			if(asset.video_data) output_start = file.get_video_length(mwindow->frame_rate);
		}
		file.seek_end();

		PluginArray *plugin_array;
		plugin_array = create_plugin_array(mwindow);
// start one plugin for each track if single channel
		if(plugin->multichannel) plugin_array->set_multichannel();

		if(realtime)
		plugin_array->start_realtime_plugins(plugin, plugin_data);
		else
		plugin_array->start_plugins(plugin);

		plugin_array->set_range(start, end);
		plugin_array->set_file(&file);

// run the effect
		if(realtime)
		{
			result = plugin_array->run_realtime_plugins(plugin->title);
			plugin_array->stop_realtime_plugins();
		}
		else
		{
			result = plugin_array->run_plugins();
			plugin_array->stop_plugins();
		}

		if(!result)
		{
			if(asset.audio_data) output_end = file.get_audio_length();
			if(asset.video_data) output_end = file.get_video_length(mwindow->frame_rate);
		}

		file.close_file();
		delete plugin_array;
	}

// paste output to tracks
	if(!result && to_tracks)
	{
		if(mwindow->gui) mwindow->gui->lock_window();
		mwindow->undo->update_undo_edits(plugin->title, 0);
// set project equal to effect sample rate and frame rates
		if(asset.audio_data)
		{
			mwindow->sample_rate = asset.rate;
			result = mwindow->paste_output(startsample, endsample, output_start, output_end, 0, 0, &asset, 0);
		}
		else
		if(asset.video_data) 
		{
// Can't change the frame rate because the frames are locked to the sample rate.
//			mwindow->frame_rate = asset.frame_rate;
			result = mwindow->paste_output(startsample, endsample, 0, 0, output_start, output_end, &asset, 0);
		}

		mwindow->undo->update_undo_edits();
		if(!result) mwindow->changes_made = 1;
		if(mwindow->gui) mwindow->gui->unlock_window();
	}
}




MenuEffectItem::MenuEffectItem(MenuEffects *menueffect, char *string)
 : BC_MenuItem(string)
{
	this->menueffect = menueffect; 
}
int MenuEffectItem::handle_event()
{
	menueffect->thread->set_title(get_text());
	menueffect->thread->start();
return 0;
}












MenuEffectWindow::MenuEffectWindow(MenuEffectThread *menueffects, ArrayList<BC_ListBoxItem*> *plugin_list, Asset *asset)
 : BC_Window("", MEGREY, ICONNAME ": Effect", plugin_list ? 580 : 420, 350, plugin_list ? 580 : 420, 350)
{ 
	this->menueffects = menueffects; 
	this->plugin_list = plugin_list; 
	this->asset = asset;
}

MenuEffectWindow::~MenuEffectWindow()
{
	delete format_tools;
}

int MenuEffectWindow::create_objects()
{
	int x;
	result = -1;

	x = 10;
// only add the list if needed
	if(plugin_list)
	{
		add_tool(new BC_Title(x, 5, "Select an effect"));
		add_tool(list = new MenuEffectWindowList(this, x, plugin_list));
		x += 160;
	}

	add_tool(new BC_Title(x, 5, "Select a file to render to:"));

	format_tools = new FormatTools(this, 
					menueffects->mwindow->plugindb, 
					asset, 
					&(asset->audio_data),
					&(asset->video_data),
					&(menueffects->dither));
	format_tools->create_objects(x, 25, 
					asset->audio_data, 
					asset->video_data, 
					0, 
					0, 
					0);
	add_tool(new BC_Title(x + 20, 250, "Overwrite project with output"));
	add_tool(to_tracks_button = new MenuEffectWindowToTracks(this, x, menueffects->to_tracks));

	add_tool(ok = new MenuEffectWindowOK(this));
	add_tool(cancel = new MenuEffectWindowCancel(this));
return 0;
}

MenuEffectWindowOK::MenuEffectWindowOK(MenuEffectWindow *window) : BC_BigButton(10, 320, "OK")
{ this->window = window; }

int MenuEffectWindowOK::handle_event() 
{ 
	if(window->plugin_list) window->result = window->list->get_selection_number(); 
	
	window->set_done(0); 
return 0;
}

int MenuEffectWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

MenuEffectWindowCancel::MenuEffectWindowCancel(MenuEffectWindow *window) : BC_BigButton(200, 320, "Cancel")
{ this->window = window; }

int MenuEffectWindowCancel::handle_event() { window->set_done(1); return 0;
}

int MenuEffectWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; } 
	return 0;
return 0;
}

MenuEffectWindowList::MenuEffectWindowList(MenuEffectWindow *window, int x, ArrayList<BC_ListBoxItem*> *plugin_list)
 : BC_ListBox(x, 30, 130, 270, plugin_list, 0, 1)
{ this->window = window; stay_highlighted(); }

int MenuEffectWindowList::handle_event() 
{
	window->result = get_selection_number();
	window->set_done(0); 
return 0;
}

MenuEffectWindowToTracks::MenuEffectWindowToTracks(MenuEffectWindow *window, int x, int default_)
 : BC_CheckBox(x, 250, 17, 17, default_) { this->window = window; }
MenuEffectWindowToTracks::~MenuEffectWindowToTracks() {}
int MenuEffectWindowToTracks::handle_event()
{
	window->menueffects->to_tracks = down;
return 0;
}




MenuEffectPrompt::MenuEffectPrompt()
 : BC_Window("", MEGREY, ICONNAME ": Effect Prompt", 256, 60, 0, 0)
{
}

MenuEffectPrompt::~MenuEffectPrompt()
{
	delete ok;
	delete cancel;
}

int MenuEffectPrompt::create_objects()
{
	add_tool(new BC_Title(5, 5, "Set up effect panel and hit \"Do it\""));
	add_tool(ok = new MenuEffectPromptOK(this));
	add_tool(cancel = new MenuEffectPromptCancel(this));
return 0;
}


MenuEffectPromptOK::MenuEffectPromptOK(MenuEffectPrompt *window)
 : BC_BigButton(5, 25, "Do it")
{
	this->window = window;
}

MenuEffectPromptOK::~MenuEffectPromptOK()
{
}

int MenuEffectPromptOK::handle_event()
{
	window->set_done(0);
return 0;
}
int MenuEffectPromptOK::keypress_event()
{
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}


MenuEffectPromptCancel::MenuEffectPromptCancel(MenuEffectPrompt *window)
 : BC_BigButton(150, 25, "Cancel")
{
	this->window = window;
}

MenuEffectPromptCancel::~MenuEffectPromptCancel()
{
}

int MenuEffectPromptCancel::handle_event()
{
	window->set_done(1);
return 0;
}
int MenuEffectPromptCancel::keypress_event()
{
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}
