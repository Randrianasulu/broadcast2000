#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

class PluginDialogCancel;
class PluginDialogDone;
class PluginDialogTextBox;
class PluginDialogDetach;
class PluginDialogAvailable;
class PluginDialogAttached;
class PluginDialogModules;
class PluginDialogAttachPlugin;
class PluginDialogAttachAttached;
class PluginDialogAttachModule;
class PluginDialogIn;
class PluginDialogOut;
class PluginDialogThru;

#include "bcbase.h"
#include "plugin.inc"
#include "mainwindow.inc"
#include "sharedpluginlocation.h"
#include "thread.h"
#include "transition.inc"

class PluginDialogThread : public Thread
{
public:
	PluginDialogThread(MainWindow *mwindow, Plugin *plugin, Transition *transition, char *title = "2000: Attach Plugin");
	~PluginDialogThread();

// Set up parameters for a transition menu.
	int set_dialog(Transition *transition, char *title);
	void run();
	MainWindow *mwindow;
	Plugin *plugin;
	Transition *transition;
	char *title;
};

class PluginDialog : public BC_Window
{
public:
	PluginDialog(char *title);
	~PluginDialog();

	int create_objects(MainWindow *mwindow, Plugin *plugin, Transition *transition);

	int attach_available(int number);
	int attach_shared(int number);
	int attach_modules(int number);
	int save_settings();

	PluginDialogCancel *cancel;
	PluginDialogDone *done;
	PluginDialogTextBox *title;
	PluginDialogDetach *detach;
	PluginDialogAvailable *available_list;
	PluginDialogAttached *attached_list;
	PluginDialogModules *module_list;
	PluginDialogAttachPlugin *available_attach;
	PluginDialogAttachAttached *attached_attach;
	PluginDialogAttachModule *module_attach;
	PluginDialogIn *in;
	PluginDialogOut *out;
	PluginDialogThru *thru;

	ArrayList<BC_ListBoxItem*> available_data;
	ArrayList<BC_ListBoxItem*> shared_data;
	ArrayList<BC_ListBoxItem*> module_data;
	ArrayList<SharedPluginLocation*> plugin_locations; // locations of shared plugins
	ArrayList<SharedModuleLocation*> module_locations; // locations of shared modules
	int selected_available;
	int selected_shared;
	int selected_modules;

// type of attached plugin
	int plugin_type;    // 0: none  1: plugin   2: shared plugin   3: module

// location of attached plugin
	SharedPluginLocation shared_plugin_location;
	SharedModuleLocation shared_module_location;

	int inoutthru;         // flag for button slide
	int new_value;         // value for button slide
	MainWindow *mwindow;
	Plugin *plugin;
	Transition *transition;
};


class PluginDialogDone : public BC_BigButton
{
public:
	PluginDialogDone(PluginDialog *dialog);
	~PluginDialogDone();
	
	int handle_event();
	int keypress_event();
	PluginDialog *dialog;
};


class PluginDialogCancel : public BC_BigButton
{
public:
	PluginDialogCancel();
	~PluginDialogCancel();
	
	int handle_event();
	int keypress_event();
};

class PluginDialogTextBox : public BC_TextBox
{
public:
	PluginDialogTextBox(char *text);
	~PluginDialogTextBox();

	int handle_event();
};

class PluginDialogDetach : public BC_BigButton
{
public:
	PluginDialogDetach(PluginDialog *dialog);
	~PluginDialogDetach();
	
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogAvailable : public BC_ListBox
{
public:
	PluginDialogAvailable(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *available_data, int x);
	~PluginDialogAvailable();
	
	int handle_event();
	int selection_changed();
	PluginDialog *dialog;
};

class PluginDialogAttached : public BC_ListBox
{
public:
	PluginDialogAttached(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *shared_data, int x);
	~PluginDialogAttached();
	
	int handle_event();
	int selection_changed();
	PluginDialog *dialog;
};

class PluginDialogModules : public BC_ListBox
{
public:
	PluginDialogModules(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *module_data, int x);
	~PluginDialogModules();
	
	int handle_event();
	int selection_changed();
	PluginDialog *dialog;
};

class PluginDialogAttachPlugin : public BC_BigButton
{
public:
	PluginDialogAttachPlugin(PluginDialog *dialog, int x);
	~PluginDialogAttachPlugin();
	
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogAttachAttached : public BC_BigButton
{
public:
	PluginDialogAttachAttached(PluginDialog *dialog, int x);
	~PluginDialogAttachAttached();
	
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogAttachModule : public BC_BigButton
{
public:
	PluginDialogAttachModule(PluginDialog *dialog, int x);
	~PluginDialogAttachModule();
	
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogIn : public BC_CheckBox
{
public:
	PluginDialogIn(PluginDialog *dialog, int setting);
	~PluginDialogIn();
	
	int button_press();
	int button_release();
	int cursor_moved_over();
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogOut : public BC_CheckBox
{
public:
	PluginDialogOut(PluginDialog *dialog, int setting);
	~PluginDialogOut();
	
	int button_press();
	int button_release();
	int cursor_moved_over();
	int handle_event();
	PluginDialog *dialog;
};

class PluginDialogThru : public BC_CheckBox
{
public:
	PluginDialogThru(PluginDialog *dialog, int setting);
	~PluginDialogThru();
	
	int button_press();
	int button_release();
	int cursor_moved_over();
	int handle_event();
	PluginDialog *dialog;
};

#endif
