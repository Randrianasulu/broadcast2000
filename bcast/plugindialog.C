#include <string.h>
#include "console.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "plugin.h"
#include "plugindialog.h"
#include "pluginserver.h"
#include "transition.h"


PluginDialogThread::PluginDialogThread(MainWindow *mwindow, Plugin *plugin, Transition *transition, char *title)
 : Thread()
{
	this->mwindow = mwindow;
	this->plugin = plugin;
	this->transition = transition;
	this->title = new char[strlen(title) + 1];
	strcpy(this->title, title);
}

PluginDialogThread::~PluginDialogThread()
{
	delete this->title;
}

int PluginDialogThread::set_dialog(Transition *transition, char *title)
{
// Set up the dialog box for a transition.
	this->transition = transition;
	this->plugin = 0;
	delete this->title;
	this->title = new char[strlen(title) + 1];
	strcpy(this->title, title);
return 0;
}

void PluginDialogThread::run()
{
	int result = 0;

	{
		PluginDialog window(title);
		window.create_objects(mwindow, plugin, transition);
		result = window.run_window();

// Done at closing
		if(!result)
		{
			AttachmentPoint *attachment;

			if(plugin)
				attachment = plugin;
			else
				attachment = transition;

			mwindow->start_reconfigure(0);

			if(attachment->plugin_type)
			{
// a plugin already is attached
// detach it
				attachment->detach(0);
			}

			if(!window.plugin_type)
			{
// plugin detached
// make it easy for user to know function of button
				attachment->update(0, 0, 0, attachment->default_title(), &window.shared_plugin_location, &window.shared_module_location);
			}
			else
			{
				attachment->update(window.plugin_type, 
					window.in->get_value(), 
					window.out->get_value(), 
					window.title->get_text(), 
					&window.shared_plugin_location, 
					&window.shared_module_location);
				attachment->attach();      // attach the new plugin
			}
			mwindow->stop_reconfigure(0);
		}
	}
}









PluginDialog::PluginDialog(char *title)
 : BC_Window("", MEGREY, title, 500, 415, 500, 415)
{
	inoutthru = 0;
}

PluginDialog::~PluginDialog()
{
	int i;
	for(i = 0; i < available_data.total; i++) delete available_data.values[i];
	available_data.remove_all();
	
	for(i = 0; i < shared_data.total; i++) delete shared_data.values[i];
	shared_data.remove_all();
	
	for(i = 0; i < module_data.total; i++) delete module_data.values[i];
	module_data.remove_all();

	for(i = 0; i < plugin_locations.total; i++) delete plugin_locations.values[i];
	plugin_locations.remove_all();

	for(i = 0; i < module_locations.total; i++) delete module_locations.values[i];
	module_locations.remove_all();

	delete title;
	delete detach;
	delete available_list;
	delete attached_list;
	delete module_list;
	delete available_attach;
	delete attached_attach;
	delete module_attach;
	delete in;
	delete out;
	delete done;
	delete cancel;
}

int PluginDialog::create_objects(MainWindow *mwindow, Plugin *plugin, Transition *transition)
{
	int use_default;
	char string[1024];
	int module_number;

	this->mwindow = mwindow;  
	this->plugin = plugin;
	this->transition = transition;

	if(plugin)
		sprintf(string, "%s: Plugin %d", plugin->get_module_title(), plugin->get_plugin_number());
	else
		sprintf(string, "Select Transition");

	add_tool(new BC_Title(5, 5, string, LARGEFONT, RED));
	add_tool(new BC_Title(10, 40, "is"));

	if((plugin && !plugin->plugin_title) ||
		(transition && !transition->plugin_title))
	{
// no plugin
		sprintf(string, "None");
	}
	else
	{
		sprintf(string, plugin ? plugin->plugin_title : transition->plugin_title);
	}

	add_tool(title = new PluginDialogTextBox(string));
	add_tool(detach = new PluginDialogDetach(this));

// GET A LIST OF ALL THE PLUGINS AVAILABLE
	int x;
	ArrayList<PluginServer*> *plugindb = mwindow->plugindb;
	for(int i = 0; i < plugindb->total; i++)
	{
		if(plugindb->values[i]->realtime)
		{
			if
			(
				(
					plugin && ((plugin->module->data_type == TRACK_AUDIO && plugindb->values[i]->audio) ||
						(plugin->module->data_type == TRACK_VIDEO && plugindb->values[i]->video))
				)
				||
				(
					transition && ((transition->audio && plugindb->values[i]->audio) ||
						(transition->video && plugindb->values[i]->video))
				)
			)
			{
				available_data.append(new BC_ListBoxItem(plugindb->values[i]->title));
			}
		}
	}


// GET A LIST OF ALL THE SHARED PLUGINS AND MODULES
// GET A LIST OF ALL THE SHARED ATTACHED PLUGINS EXCEPT FOR THIS ONE
// GET A LIST OF ALL THE MODULES
	if(plugin) module_number = mwindow->console->modules->number_of(plugin->module);
	else
		module_number = -1;

	if((plugin && plugin->module->data_type == TRACK_AUDIO) ||
		(transition && transition->audio))
	{
		mwindow->console->modules->shared_aplugins(&shared_data, &plugin_locations, module_number);
		mwindow->console->modules->shared_amodules(&module_data, &module_locations, module_number);
	}
	else
	if((plugin && plugin->module->data_type == TRACK_VIDEO) ||
		(transition && transition->video))
	{
		mwindow->console->modules->shared_vplugins(&shared_data, &plugin_locations, module_number);
		mwindow->console->modules->shared_vmodules(&module_data, &module_locations, module_number);
	}

	x = 10;
	add_tool(new BC_Title(x, 65, "Plugins"));
	add_tool(available_list = new PluginDialogAvailable(this, &available_data, x));
	add_tool(available_attach = new PluginDialogAttachPlugin(this, x));

	x = 170;
	add_tool(new BC_Title(x, 65, "Shared plugins"));
	add_tool(attached_list = new PluginDialogAttached(this, &shared_data, x));
	add_tool(attached_attach = new PluginDialogAttachAttached(this, x + 20));

	x = 320;
	add_tool(new BC_Title(x, 65, "Shared modules"));
	add_tool(module_list = new PluginDialogModules(this, &module_data, x));
	add_tool(module_attach = new PluginDialogAttachModule(this, x));
	
// assume user wants in and out
	if((plugin && plugin->in == 0 && plugin->out == 0) ||
		(transition && transition->in == 0 && transition->out == 0))
		use_default = 1;
	else
		use_default = 0;
	
	add_tool(in = new PluginDialogIn(this, use_default ? 1 : (plugin ? plugin->in : transition->in)));
	add_tool(out = new PluginDialogOut(this, use_default ? 1 : (plugin ? plugin->out : transition->out)));
	add_tool(done = new PluginDialogDone(this));
	add_tool(cancel = new PluginDialogCancel);

	if(plugin)
	{	
		this->shared_plugin_location = plugin->shared_plugin_location;
		this->shared_module_location = plugin->shared_module_location;
		this->plugin_type = plugin->plugin_type;
	}
	else
	if(transition)
	{
		this->shared_plugin_location = transition->shared_plugin_location;
		this->shared_module_location = transition->shared_module_location;
		this->plugin_type = transition->plugin_type;
	}

	selected_available = -1;
	selected_shared = -1;
	selected_modules = -1;
return 0;
}

int PluginDialog::attach_available(int number)
{
	if(number > -1 && number < available_data.total) 
	{
		title->update(available_data.values[number]->text);
		plugin_type = 1;         // type is plugin
	}
return 0;
}

int PluginDialog::attach_shared(int number)
{
	if(number > -1 && number < shared_data.total) 
	{
		title->update(shared_data.values[number]->text);
		plugin_type = 2;         // type is shared plugin
		shared_plugin_location = *(plugin_locations.values[number]); // copy location
	}
return 0;
}

int PluginDialog::attach_modules(int number)
{
	if(number > -1 && number < module_data.total) 
	{
		title->update(module_data.values[number]->text);
		plugin_type = 3;         // type is module
		shared_module_location = *(module_locations.values[number]); // copy location
	}
return 0;
}

int PluginDialog::save_settings()
{
return 0;
}








PluginDialogDone::PluginDialogDone(PluginDialog *dialog)
 : BC_BigButton(10, 380, "Do it") { this->dialog = dialog; }
PluginDialogDone::~PluginDialogDone() { }
int PluginDialogDone::handle_event()
{
	set_done(0); 
return 0;
}
int PluginDialogDone::keypress_event()
{
	if(get_keypress() == 13)
	{
		handle_event();
		return 1;
	}
	return 0;
return 0;
}

PluginDialogCancel::PluginDialogCancel()
 : BC_BigButton(340, 380, "Cancel") { }
PluginDialogCancel::~PluginDialogCancel() { }
int PluginDialogCancel::handle_event()
{ set_done(1); return 0;
}
int PluginDialogCancel::keypress_event()
{
	if(get_keypress() == ESC)
	{
		handle_event();
		return 1;
	}
	return 0;
return 0;
}

PluginDialogTextBox::PluginDialogTextBox(char *text)
 : BC_TextBox(30, 35, 200, text) { }
PluginDialogTextBox::~PluginDialogTextBox() { }
int PluginDialogTextBox::handle_event() { return 0;
}

PluginDialogDetach::PluginDialogDetach(PluginDialog *dialog)
 : BC_BigButton(240, 35, "Detach") { this->dialog = dialog; }
PluginDialogDetach::~PluginDialogDetach() { }
int PluginDialogDetach::handle_event() 
{
	dialog->title->update("None");
	dialog->plugin_type = 0;         // type is none
return 0;
}

PluginDialogAvailable::PluginDialogAvailable(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *available_data, int x)
 : BC_ListBox(x, 85, 130, 200, available_data, 0, 1) 
{ this->dialog = dialog; stay_highlighted(); }
PluginDialogAvailable::~PluginDialogAvailable() { }
int PluginDialogAvailable::handle_event() 
{ 
	dialog->attach_available(get_selection_number()); 
	deactivate();

	set_done(0); 
return 0;
}
int PluginDialogAvailable::selection_changed()
{
	dialog->selected_available = get_selection_number();
return 0;
}

PluginDialogAttached::PluginDialogAttached(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *shared_data, int x)
 : BC_ListBox(x, 85, 130, 200, shared_data, 0, 1) 
{ this->dialog = dialog; stay_highlighted(); }
PluginDialogAttached::~PluginDialogAttached() { }
int PluginDialogAttached::handle_event()
{ 
	dialog->attach_shared(get_selection_number()); 
	deactivate();

	set_done(0); 
return 0;
}
int PluginDialogAttached::selection_changed()
{
	dialog->selected_shared = get_selection_number();
return 0;
}

PluginDialogModules::PluginDialogModules(PluginDialog *dialog, ArrayList<BC_ListBoxItem*> *module_data, int x)
 : BC_ListBox(x, 85, 150, 200, module_data, 0, 1) 
{ this->dialog = dialog; stay_highlighted(); }
PluginDialogModules::~PluginDialogModules() { }
int PluginDialogModules::handle_event()
{ 
	dialog->attach_modules(get_selection_number()); 
	deactivate();

	set_done(0); 
return 0;
}
int PluginDialogModules::selection_changed()
{
	dialog->selected_modules = get_selection_number();
return 0;
}

PluginDialogAttachPlugin::PluginDialogAttachPlugin(PluginDialog *dialog, int x)
 : BC_BigButton(x, 310, "Attach") { this->dialog = dialog; }
PluginDialogAttachPlugin::~PluginDialogAttachPlugin() { }
int PluginDialogAttachPlugin::handle_event() 
{ 
	dialog->attach_available(dialog->selected_available); 
return 0;
}

PluginDialogAttachAttached::PluginDialogAttachAttached(PluginDialog *dialog, int x)
 : BC_BigButton(x, 310, "Attach") { this->dialog = dialog; }
PluginDialogAttachAttached::~PluginDialogAttachAttached() { }
int PluginDialogAttachAttached::handle_event() 
{ 
	dialog->attach_shared(dialog->selected_shared); 
return 0;
}


PluginDialogAttachModule::PluginDialogAttachModule(PluginDialog *dialog, int x)
 : BC_BigButton(x, 310, "Attach") { this->dialog = dialog; }
PluginDialogAttachModule::~PluginDialogAttachModule() { }
int PluginDialogAttachModule::handle_event() 
{ 
	dialog->attach_modules(dialog->selected_modules); 
return 0;
}


PluginDialogIn::PluginDialogIn(PluginDialog *dialog, int setting)
 : BC_CheckBox(100, 350, 17, 17, setting, "Send") { this->dialog = dialog; }
PluginDialogIn::~PluginDialogIn() { }
int PluginDialogIn::handle_event() { return 0;
}
int PluginDialogIn::button_press()
{
	dialog->inoutthru = 1;
	dialog->new_value = get_value();
	return 1;
return 0;
}
int PluginDialogIn::button_release()
{
	if(dialog->inoutthru) dialog->inoutthru = 0;
return 0;
}
int PluginDialogIn::cursor_moved_over()
{
	if(dialog->inoutthru && get_value() != dialog->new_value)
	{
		update(dialog->new_value);
	}
return 0;
}

PluginDialogOut::PluginDialogOut(PluginDialog *dialog, int setting)
 : BC_CheckBox(200, 350, 17, 17, setting, "Receive") { this->dialog = dialog; }
PluginDialogOut::~PluginDialogOut() { }
int PluginDialogOut::handle_event() { return 0;
}
int PluginDialogOut::button_press()
{
	dialog->inoutthru = 1;
	dialog->new_value = get_value();
	return 1;
return 0;
}
int PluginDialogOut::button_release()
{
	if(dialog->inoutthru) dialog->inoutthru = 0;
return 0;
}
int PluginDialogOut::cursor_moved_over()
{
	if(dialog->inoutthru && get_value() != dialog->new_value)
	{
		update(dialog->new_value);
	}
return 0;
}

PluginDialogThru::PluginDialogThru(PluginDialog *dialog, int setting)
 : BC_CheckBox(300, 350, 17, 17, setting, "Thru") { this->dialog = dialog; }
PluginDialogThru::~PluginDialogThru() { }
int PluginDialogThru::handle_event() { return 0;
}
int PluginDialogThru::button_press()
{
	dialog->inoutthru = 1;
	dialog->new_value = get_value();
	return 1;
return 0;
}
int PluginDialogThru::button_release()
{
	if(dialog->inoutthru) dialog->inoutthru = 0;
return 0;
}
int PluginDialogThru::cursor_moved_over()
{
	if(dialog->inoutthru && get_value() != dialog->new_value)
	{
		update(dialog->new_value);
	}
return 0;
}
