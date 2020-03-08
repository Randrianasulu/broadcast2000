#include <string.h>
#include "assets.h"
#include "audiodevice.inc"
#include "cache.h"
#include "console.h"
#include "filesystem.h"
#include "indexprefs.h"
#include "levelwindow.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "modules.h"
#include "playbackengine.h"
#include "playbackprefs.h"
#include "pluginprefs.h"
#include "preferences.h"
#include "recordprefs.h"
#include "viewprefs.h"
#include "videoprefs.h"

PreferencesMenuitem::PreferencesMenuitem(MainWindow *mwindow)
 : BC_MenuItem("Preferences...", "")
{
	this->mwindow = mwindow; 
	thread = new PreferencesThread(mwindow);
}

PreferencesMenuitem::~PreferencesMenuitem()
{
	delete thread;
}


int PreferencesMenuitem::handle_event() 
{
	if(thread->thread_running) return 1;
	thread->start();
	return 1;
return 0;
}




PreferencesThread::PreferencesThread(MainWindow *mwindow)
 : Thread()
{
	this->mwindow = mwindow;
	window = 0;
	thread_running = 0;
}

PreferencesThread::~PreferencesThread()
{
}

void PreferencesThread::run()
{
	int need_new_indexes;

	current_dialog = mwindow->defaults->get("DEFAULTPREF", 0);

	window = new PreferencesWindow(mwindow, this);
	window->initialize();
	thread_running = 1;
	window->run_window();
	thread_running = 0;
	need_new_indexes = window->redraw_indexes;
	delete window;
	window = 0;

	mwindow->defaults->update("DEFAULTPREF", current_dialog);

// indexes were deleted
	if(need_new_indexes)
	{
// create new index files
		mwindow->assets->build_indexes();
// draw
		mwindow->draw();
	}
}

int PreferencesThread::update_framerate()
{
	if(thread_running && window)
	{
		window->update_framerate();
	}
	return 0;
return 0;
}












PreferencesWindow::PreferencesWindow(MainWindow *mwindow, PreferencesThread *thread)
 : BC_Window("", MEGREY, ICONNAME ": Preferences", 440 + 160, 350 + 80, 0, 0)
{
	this->mwindow = mwindow;
	this->thread = thread;
	preferences = new Preferences;
	redraw_indexes = 0;
	redraw_meters = 0;
	redraw_times = 0;
	close_assets = 0;
	reload_plugins = 0;
	dialog = 0;
}

PreferencesWindow::~PreferencesWindow()
{
	if(dialog) delete dialog;
	delete categorywindow;
	delete commandwindow;
	delete preferences;
}

int PreferencesWindow::initialize()
{
	*preferences = *mwindow->preferences;
	add_subwindow(categorywindow = new CategoryWindow(this));
	add_subwindow(commandwindow = new CommandWindow(this));
	set_current_dialog(thread->current_dialog);
	return 0;
return 0;
}

int PreferencesWindow::update_framerate()
{
	lock_window();
	if(thread->current_dialog == 5)
	{
		preferences->actual_frame_rate = mwindow->preferences->actual_frame_rate;
		dialog->draw_framerate();
		flash();
	}
	unlock_window();
	return 0;
return 0;
}

int PreferencesWindow::set_current_dialog(int number)
{
	thread->current_dialog = number;
	if(dialog) delete dialog;
	dialog = 0;

	switch(number)
	{
		case 0:
			add_subwindow(dialog = new PlaybackPrefs(this));
			break;
	
		case 1:
			add_subwindow(dialog = new RecordPrefs(this));
			break;
	
		case 2:
			add_subwindow(dialog = new IndexPrefs(this));
			break;
	
		case 3:
			add_subwindow(dialog = new ViewPrefs(this));
			break;
	
		case 4:
			add_subwindow(dialog = new PluginPrefs(this));
			break;
	
		case 5:
			add_subwindow(dialog = new VideoPrefs(this));
			break;
	}
	return 0;
return 0;
}

// ================================== save values
int PreferencesWindow::save_values()
{
	*mwindow->preferences = *preferences;
	if(redraw_meters) 
	{
		mwindow->level_window->change_format();
		mwindow->console->modules->change_format();
	}

	if(redraw_times) mwindow->redraw_time_dependancies();
	return 0;
return 0;
}




PreferencesDialog::PreferencesDialog(PreferencesWindow *pwindow)
 : BC_SubWindow(150, 10, 440, 350)
{
	this->pwindow = pwindow;
}

PreferencesDialog::~PreferencesDialog()
{
}

// ============================== category window




CategoryWindow::CategoryWindow(PreferencesWindow *pwindow)
 : BC_SubWindow(10, 10, 130, 350, MEGREY)
{ 
	this->pwindow = pwindow; 
}

CategoryWindow::~CategoryWindow()
{
	delete playbackbutton;
	delete recordbutton;
	delete indexbutton;
	delete viewbutton;
	delete pluginbutton;
}

int CategoryWindow::create_objects()
{
	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(new BC_Title(5, 5, "Categories", LARGEFONT, BLACK));
	add_tool(playbackbutton = new PlaybackButton(pwindow));
	add_tool(recordbutton = new RecordButton(pwindow));
	add_tool(indexbutton = new IndexButton(pwindow));
	add_tool(viewbutton = new ViewButton(pwindow));
	add_tool(pluginbutton = new PluginPrefsButton(pwindow));
	add_tool(new VideoPrefsButton(pwindow));
return 0;
}



// ================================= command window





CommandWindow::CommandWindow(PreferencesWindow *pwindow)
 : BC_SubWindow(10, 350 + 20, 580, 50, MEGREY)
{
	this->pwindow = pwindow;
}

CommandWindow::~CommandWindow()
{
	delete ok;
	delete cancel;
}

int CommandWindow::create_objects()
{
	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(ok = new OkButton(pwindow));
	add_tool(cancel = new PrefCancelButton(pwindow));
return 0;
}

OkButton::OkButton(PreferencesWindow *pwindow)
 : BC_BigButton(10, 10, "OK")
{
	this->pwindow = pwindow;
}

OkButton::~OkButton()
{
}

int OkButton::handle_event()
{
	pwindow->save_values();
	pwindow->set_done(0);
return 0;
}

int OkButton::keypress_event()
{
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

PrefCancelButton::PrefCancelButton(PreferencesWindow *pwindow)
 : BC_BigButton(300, 10, "Cancel")
{
	this->pwindow = pwindow;
}

PrefCancelButton::~PrefCancelButton()
{
}

int PrefCancelButton::handle_event()
{
	pwindow->set_done(1);
return 0;
}

int PrefCancelButton::keypress_event()
{
	if(get_keypress() == ESC) { handle_event();  return 1; }
	return 0;
return 0;
}






// ==================================== category buttons






PlaybackButton::PlaybackButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 40, "Audio Out")
{ this->pwindow = pwindow; }
PlaybackButton::~PlaybackButton() {}
int PlaybackButton::handle_event()
{ pwindow->set_current_dialog(0); return 0;
}

RecordButton::RecordButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 70, "Audio In")
{ this->pwindow = pwindow; }
RecordButton::~RecordButton()
{ }
int RecordButton::handle_event()
{ pwindow->set_current_dialog(1); return 0;
}

IndexButton::IndexButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 100, "Performance")
{ this->pwindow = pwindow; }
IndexButton::~IndexButton()
{ }
int IndexButton::handle_event()
{ pwindow->set_current_dialog(2); return 0;
}

ViewButton::ViewButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 130, "Interface")
{ this->pwindow = pwindow; }
ViewButton::~ViewButton()
{ }
int ViewButton::handle_event()
{ pwindow->set_current_dialog(3); return 0;
}

PluginPrefsButton::PluginPrefsButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 160, "Plugins")
{ this->pwindow = pwindow; }
PluginPrefsButton::~PluginPrefsButton()
{ }
int PluginPrefsButton::handle_event()
{ pwindow->set_current_dialog(4); return 0;
}

VideoPrefsButton::VideoPrefsButton(PreferencesWindow *pwindow)
 : BC_BigButton(5, 190, "Video")
{ this->pwindow = pwindow; }
VideoPrefsButton::~VideoPrefsButton()
{ }
int VideoPrefsButton::handle_event()
{ pwindow->set_current_dialog(5); return 0;
}
