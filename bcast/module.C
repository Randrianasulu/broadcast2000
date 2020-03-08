#include <string.h>
#include "console.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "patch.h"
#include "patchbay.h"
#include "plugin.h"
#include "pluginserver.h"
#include "sharedpluginlocation.h"
#include "track.h"
#include "tracks.h"

Module::Module(MainWindow *mwindow)
 : ListItem<Module>()
{
	this->mwindow = mwindow;
	console = mwindow->console;
	modules = console->modules;
	mute = 0;
}

Module::~Module()
{
}

Patch* Module::get_patch_of()
{
	return mwindow->patches->number(modules->number_of(this));
}

Track* Module::get_track_of()
{
	return mwindow->tracks->number(modules->number_of(this));
}

int Module::render_init(int realtime_sched, int duplicate)
{
	int i;
	for(i = 0; i < PLUGINS; i++)
	{
		plugins[i]->render_init(realtime_sched, duplicate);
	}
return 0;
}

int Module::render_stop(int duplicate)
{
	int i;
	for(i = 0; i < PLUGINS; i++)
	{
		plugins[i]->render_stop(duplicate);
	}
return 0;
}

int Module::swap_plugins(int number1, int number2)
{
//printf("Module::swap_plugins %d %d\n", number1, number2);
	for(int i = 0; i < PLUGINS; i++)
	{
		if(plugins[i]->plugin_type == 2)
		{
			if(plugins[i]->shared_plugin_location.module == number1)
				plugins[i]->shared_plugin_location.module = number2;
			else
			if(plugins[i]->shared_plugin_location.module == number2)
				plugins[i]->shared_plugin_location.module = number1;
		}

		if(plugins[i]->plugin_type == 3)
		{
//printf("Module::swap_plugins 1 %d\n", plugins[i]->shared_module_location.module);
			if(plugins[i]->shared_module_location.module == number1)
				plugins[i]->shared_module_location.module = number2;
			else
			if(plugins[i]->shared_module_location.module == number2)
				plugins[i]->shared_module_location.module = number1;
//printf("Module::swap_plugins 2 %d\n", plugins[i]->shared_module_location.module);
		}
	}
return 0;
}


int Module::shift_module_pointers(int deleted_track)
{
	for(int i = 0; i < PLUGINS; i++)
	{
		if(plugins[i]->plugin_type == 2)
		{
			if(plugins[i]->shared_plugin_location.module > deleted_track)
				plugins[i]->shared_plugin_location.module--;
			else
			if(plugins[i]->shared_plugin_location.module == deleted_track)
				plugins[i]->detach();
		}

		if(plugins[i]->plugin_type == 3)
		{
			if(plugins[i]->shared_module_location.module > deleted_track)
				plugins[i]->shared_module_location.module--;
			else
			if(plugins[i]->shared_module_location.module == deleted_track)
				plugins[i]->detach();
		}
	}
	return 0;
return 0;
}

int Module::shared_plugins(ArrayList<BC_ListBoxItem*> *shared_data, ArrayList<SharedPluginLocation*> *plugin_locations)
{
	int this_number = modules->number_of(this);
	char string[1024];

	for(int i = 0; i < PLUGINS; i++)
	{
		if(plugins[i]->plugin_server)
		{
// add all plugins
// put its title in the string
			sprintf(string, "%s: #%d", title, plugins[i]->get_plugin_number());

			shared_data->append(new BC_ListBoxItem(string));
			plugin_locations->append(new SharedPluginLocation(this_number, i));
		}
	}
return 0;
}

int Module::toggles_selected(int on, int show, int mute)
{
	int total = 0;
	for(int i = 0; i < PLUGINS; i++)
	{
		if(on && plugins[i]->on) total++;
		if(show && plugins[i]->show) total++;
		if(mute && this->mute) total++;
	}
	return total;
return 0;
}

int Module::select_all_toggles(int on, int show, int mute)
{
	for(int i = 0; i < PLUGINS; i++)
	{
		if(on && !plugins[i]->on)
		{
			plugins[i]->on = 1;
			if(console->gui) plugins[i]->on_toggle->update(plugins[i]->on);
		}
		if(show && !plugins[i]->show)
		{
			plugins[i]->show = 1;
			if(console->gui) plugins[i]->show_toggle->update(plugins[i]->show);
			if(plugins[i]->plugin_server)
			{
				plugins[i]->show_gui();
			}
		}
		if(mute && !this->mute)
		{
			this->mute = 1;
			if(console->gui) set_mute(this->mute);
		}
	}
	return 0;
return 0;
}

int Module::deselect_all_toggles(int on, int show, int mute)
{
	for(int i = 0; i < PLUGINS; i++)
	{
		if(on && plugins[i]->on) 
		{
			plugins[i]->on = 0;
			if(console->gui) plugins[i]->on_toggle->update(plugins[i]->on);
		}
		if(show && plugins[i]->show) 
		{
			plugins[i]->show = 0;
			if(console->gui) plugins[i]->show_toggle->update(plugins[i]->show);
			if(plugins[i]->plugin_server)
			{
				plugins[i]->hide_gui();
			}
		}
		if(mute && this->mute) 
		{
			this->mute = 0;
			if(console->gui) set_mute(this->mute);
		}
	}
	return 0;
return 0;
}

int Module::console_routing_used()
{
	int i;
	for(i = 0; i < PLUGINS; i++)
	{
		if(plugins[i]->on && plugins[i]->plugin_type) return 1;
	}
	return 0;
return 0;
}

int Module::console_adjusting_used()
{
	if(fade != 100 || mute) return 1;
	else
	return 0;
return 0;
}


