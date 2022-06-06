#include <string.h>
#include "console.h"
#include "mainwindow.h"
#include "plugin.h"
#include "plugindialog.h"
#include "pluginpopup.h"


PluginPopup::PluginPopup(Plugin *plugin, int x, int y)
 : BC_PopupMenu(x, y, 100, plugin->plugin_title, 1)
{
	this->plugin = plugin;
}

PluginPopup::~PluginPopup()
{
	delete attach;
	delete in;
	delete out;
}

int PluginPopup::update(int in, int out, const char* title)
{
	if(title)
		BC_PopupMenu::update(title);
	else
		BC_PopupMenu::update("Plugin");

	this->in->set_checked(in);
	this->out->set_checked(out);
return 0;
}


int PluginPopup::add_items()
{
	add_item(attach = new PluginPopupAttach(plugin));
	add_item(in = new PluginPopupIn(plugin));
	add_item(out = new PluginPopupOut(plugin));
//	add_item(thru = new PluginPopupThru(plugin));
	add_item(detach = new PluginPopupDetach(plugin));
return 0;
}

int PluginPopup::handle_event()
{
return 0;
}








PluginPopupAttach::PluginPopupAttach(Plugin *plugin)
 : BC_PopupItem("Attach...")
{
	this->plugin = plugin;
	dialog_thread = new PluginDialogThread(plugin->mwindow, plugin, 0, ICONNAME ": Plugin");
}

PluginPopupAttach::~PluginPopupAttach()
{
	delete dialog_thread;
}

int PluginPopupAttach::handle_event()
{
	dialog_thread->start();
return 0;
}







PluginPopupDetach::PluginPopupDetach(Plugin *plugin)
 : BC_PopupItem("Detach")
{
	this->plugin = plugin;
}

PluginPopupDetach::~PluginPopupDetach()
{
}

int PluginPopupDetach::handle_event()
{
	if(plugin->plugin_type)
	{
// a plugin already is attached
		plugin->mwindow->console->start_reconfigure(1);
// detach it
		plugin->detach();			
// make it easy for user to know function of button
		plugin->update(0, 0, 0, "Plugin", 0, 0);
		plugin->mwindow->console->stop_reconfigure(1);
	}
return 0;
}







PluginPopupIn::PluginPopupIn(Plugin *plugin)
 : BC_PopupItem("Send")
{
	this->plugin = plugin;
	checked = plugin->in;
}

PluginPopupIn::~PluginPopupIn()
{
}

int PluginPopupIn::handle_event()
{
	plugin->mwindow->console->start_reconfigure(1);
	plugin->in ^= 1;
	checked = plugin->in;
	plugin->mwindow->console->stop_reconfigure(1);
return 0;
}





PluginPopupOut::PluginPopupOut(Plugin *plugin)
 : BC_PopupItem("Receive")
{
	this->plugin = plugin;
	checked = plugin->out;
}

PluginPopupOut::~PluginPopupOut()
{
}

int PluginPopupOut::handle_event()
{
	plugin->mwindow->console->start_reconfigure(1);
	plugin->out ^= 1;
	checked = plugin->out;
	plugin->mwindow->console->stop_reconfigure(1);
return 0;
}




// PluginPopupThru::PluginPopupThru(Plugin *plugin)
//  : BC_PopupItem("Thru")
// {
// 	this->plugin = plugin;
// 	checked = plugin->thru;
// }
// 
// PluginPopupThru::~PluginPopupThru()
// {
// }
// 
// int PluginPopupThru::handle_event()
// {
// 	plugin->mwindow->console->start_reconfigure(1);
// 	plugin->thru ^= 1;
// 	checked = plugin->thru;
// 	plugin->mwindow->console->stop_reconfigure(1);
// }
