#ifndef PLUGIN_H
#define PLUGIN_H

#include "attachmentpoint.h"
#include "bcbase.h"
#include "console.inc"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "module.inc"
#include "pluginbuffer.inc"
#include "sharedpluginlocation.h"
#include "plugin.inc"
#include "pluginpopup.inc"
#include "pluginserver.inc"
#include "virtualnode.inc"

class PluginOnToggle;

class Plugin : public AttachmentPoint
{
public:
	Plugin(MainWindow *mwindow, Module *module, int plugin_number);
	virtual ~Plugin();

	int update_derived();

	int get_plugin_number();        // get the number of this plugin in the module

// swap module numbers when moving tracks
	int swap_modules(int number1, int number2);
	int set_show_derived(int value);
	int set_string();     // set the string that appears on the plugin
	virtual int use_gui() { return 0; };       // whether or not the module has a gui
	char* get_module_title();
	int resize_plugin(int x, int y);
// Update the widgets after loading
	int update_display();
	char* default_title();

// settings for plugin
	int plugin_number;          // number of this plugin in the module starting with 1

	MainWindow *mwindow;
	PluginPopup *plugin_popup;
	PluginShowToggle *show_toggle;
	PluginOnToggle *on_toggle;
	BC_Title *show_title;
	BC_Title *on_title;

//	char title[1024];           // title of the plugin currently attached
	Module *module;
};



class PluginShowToggle : public BC_Radial
{
public:
	PluginShowToggle(Plugin *plugin, Console *console, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();

	Console *console;
	Plugin *plugin;
};

class PluginOnToggle : public BC_Radial
{
public:
	PluginOnToggle(Plugin *plugin, Console *console, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();

	Console *console;
	Plugin *plugin;
};

/* class PluginSoloToggle : public BC_Radial
{
public:
	PluginSoloToggle();
	~PluginSoloToggle();

	int handle_event();
};
 */




#endif
