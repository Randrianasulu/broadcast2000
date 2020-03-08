#include <string.h>
#include "console.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "messages.h"
#include "module.h"
#include "plugin.h"
#include "pluginbuffer.h"
#include "pluginpopup.h"
#include "pluginserver.h"
#include "virtualnode.h"




Plugin::Plugin(MainWindow *mwindow, Module *module, int plugin_number)
 : AttachmentPoint(mwindow)
{
	this->mwindow = mwindow;
	this->plugin_number = plugin_number;
	this->module = module;
	plugin_server = 0;
	in = out = 0;
	on = 1;
	show = 1;
	plugin_type = 0;
	sprintf(plugin_title, default_title());
	plugin_popup = 0;
	show_title = 0;
	show_toggle = 0;
	on_toggle = 0;
	on_title = 0;
	total_input_buffers = 0;
	new_total_input_buffers = 0;
}

Plugin::~Plugin()
{
//	detach();

	if(use_gui())
	{
		delete on_toggle;
		delete on_title;
		delete show_toggle;
		delete show_title;
		delete plugin_popup;
	}

//	if(plugin_server) delete plugin_server;
//	plugin_server = 0;
}

char* Plugin::default_title()
{
	return "Plugin";
}

int Plugin::update_derived()
{
	plugin_popup->update(in, out, plugin_title);
return 0;
}

int Plugin::update_display()
{
	if(mwindow->gui)
	{
		plugin_popup->update(in, out, plugin_title);
		show_toggle->update(show);
		on_toggle->update(on);
	}
return 0;
}

int Plugin::swap_modules(int number1, int number2)
{
	if(shared_plugin_location.module == number1) shared_plugin_location.module == number2;
	else
	if(shared_plugin_location.module == number2) shared_plugin_location.module == number1;

	if(shared_module_location.module == number1) shared_module_location.module == number2;
	else
	if(shared_module_location.module == number2) shared_module_location.module == number1;
return 0;
}

int Plugin::set_show_derived(int value)
{
	if(show_toggle) show_toggle->update(value);
return 0;
}

int Plugin::set_string()
{
	if(plugin_server)
	{
		if(use_gui())
		{
			char new_string[1024];
			sprintf(new_string, "%s: %s\n", get_module_title(), plugin_title);
			plugin_server->set_string(new_string);
		}
	}
return 0;
}

char* Plugin::get_module_title()
{
	return module->title;
}

int Plugin::resize_plugin(int x, int y)
{
	if(plugin_popup) plugin_popup->resize_tool(x, y);
	if(show_toggle) show_toggle->resize_tool(x + 10, y + 23);
	if(show_title) show_title->resize_tool(x + 30, y + 25);
	if(on_toggle) on_toggle->resize_tool(x + 60, y + 23);
	if(on_title) on_title->resize_tool(x + 80, y + 25);
return 0;
}


int Plugin::get_plugin_number()
{
	return plugin_number;
return 0;
}



// ======================================= radial

PluginShowToggle::PluginShowToggle(Plugin *plugin, Console *console, int x, int y)
 : BC_Radial(x, y, 16, 16, plugin->show)
{
	this->console = console;
	this->plugin = plugin;
}

int PluginShowToggle::handle_event()
{
	if(shift_down())
	{
		int total_selected = console->toggles_selected(0, 1, 0);

		if(total_selected == 0)
		{
// nothing previously selected
			console->select_all_toggles(0, 1, 0);
		}
		else
		if(total_selected == 1)
		{
			if(plugin->show)
			{
// this patch was previously the only one on
				console->select_all_toggles(0, 1, 0);
			}
			else
			{
// another patch was previously the only one on
				console->deselect_all_toggles(0, 1, 0);
				plugin->show = 1;
			}
		}
		else
		if(total_selected > 1)
		{
// other patches were previously on
			console->deselect_all_toggles(0, 1, 0);
			plugin->show = 1;
		}
		
		update(plugin->show);
	}
	else
	{
		if(get_value() != plugin->show)
		{
			plugin->show = get_value();
			if(plugin->plugin_server)
			{
				if(get_value()) plugin->show_gui();
				else plugin->hide_gui();
			}
		}
	}

	console->button_down = 1;
	console->new_status = get_value();
return 0;
}

int PluginShowToggle::cursor_moved_over()
{
	if(console->button_down && console->new_status != get_value())
	{
		update(console->new_status);
		plugin->show = get_value();

		if(plugin->plugin_server && console->gui)
		{
			if(get_value()) plugin->show_gui();
			else plugin->hide_gui();
		}
	}
return 0;
}

int PluginShowToggle::button_release()
{
	console->button_down = 0;
return 0;
}

PluginOnToggle::PluginOnToggle(Plugin *plugin, Console *console, int x, int y)
 : BC_Radial(x, y, 16, 16, plugin->show)
{
	this->console = console;
	this->plugin = plugin;
}

int PluginOnToggle::handle_event()
{
	if(shift_down())
	{
		int total_selected = console->toggles_selected(1, 0, 0);

		if(total_selected == 0)
		{
// nothing previously selected
			console->select_all_toggles(1, 0, 0);
		}
		else
		if(total_selected == 1)
		{
			if(plugin->on)
			{
// this patch was previously the only one on
				console->select_all_toggles(1, 0, 0);
			}
			else
			{
// another patch was previously the only one on
				console->deselect_all_toggles(1, 0, 0);
				plugin->on = 1;
			}
		}
		else
		if(total_selected > 1)
		{
// other patches were previously on
			console->deselect_all_toggles(1, 0, 0);
			plugin->on = 1;
		}

		update(plugin->on);
	}
	else
	{
		if(/*plugin->plugin_server && */get_value() != plugin->on)
		{
			plugin->on = get_value();
		}
	}

	console->button_down = 1;
	console->reconfigure_trigger = 1;
	console->new_status = get_value();
	return 0;
return 0;
}

int PluginOnToggle::cursor_moved_over()
{
	if(console->button_down && console->new_status != get_value())
	{
		update(console->new_status);
		plugin->on = get_value();
	}
return 0;
}

int PluginOnToggle::button_release()
{
	console->button_down = 0;
return 0;
}
