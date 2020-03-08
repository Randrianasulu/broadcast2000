#ifndef PREFERENCESTHREAD_H
#define PREFERENCESTHREAD_H

class CategoryWindow;
class CommandWindow;

#include "bcbase.h"
#include "indexprefs.inc"
#include "mainwindow.inc"
#include "playbackprefs.inc"
#include "pluginprefs.inc"
#include "preferences.inc"
#include "preferencesthread.inc"
#include "recordprefs.inc"
#include "thread.h"
#include "viewprefs.inc"
#include "videoprefs.inc"


class PreferencesMenuitem : public BC_MenuItem
{
public:
	PreferencesMenuitem(MainWindow *mwindow);
	~PreferencesMenuitem();

	int handle_event();

	MainWindow *mwindow;
	PreferencesThread *thread;
};

class PreferencesThread : public Thread
{
public:
	PreferencesThread(MainWindow *mwindow);
	~PreferencesThread();
	void run();

	int update_framerate();

	int current_dialog;
	int thread_running;
	PreferencesWindow *window;
	MainWindow *mwindow;
};

class PreferencesDialog : public BC_SubWindow
{
public:
	PreferencesDialog(PreferencesWindow *pwindow);
	virtual ~PreferencesDialog();
	PreferencesWindow *pwindow;
	
	virtual int draw_framerate() { return 0; };
};

class PreferencesWindow : public BC_Window
{
public:
	PreferencesWindow(MainWindow *mwindow, PreferencesThread *thread);
	~PreferencesWindow();

	int initialize();
	int save_values();
	int delete_current_dialog();
	int set_current_dialog(int number);
	int update_framerate();

	MainWindow *mwindow;
	PreferencesThread *thread;
// Copy of mainwindow preferences
	Preferences *preferences;

	int redraw_indexes;
	int redraw_meters;
	int redraw_times;
	int close_assets;
	int reload_plugins;

private:
	CategoryWindow *categorywindow;
	CommandWindow *commandwindow;
	PreferencesDialog *dialog;
};

class PlaybackButton;
class RecordButton;
class IndexButton;
class ViewButton;
class PluginPrefsButton;
class VideoPrefsButton;

class CategoryWindow : public BC_SubWindow
{
public:
	CategoryWindow(PreferencesWindow *pwindow);
	~CategoryWindow();
	
	int create_objects();
	PreferencesWindow *pwindow;
	
	PlaybackButton *playbackbutton;
	RecordButton *recordbutton;
	IndexButton *indexbutton;
	ViewButton *viewbutton;
	PluginPrefsButton *pluginbutton;
};

class OkButton;
class PrefCancelButton;

class CommandWindow : public BC_SubWindow
{
public:
	CommandWindow(PreferencesWindow *pwindow);
	~CommandWindow();
	
	int create_objects();
	PreferencesWindow *pwindow;
	OkButton *ok;
	PrefCancelButton *cancel;
};

class OkButton : public BC_BigButton
{
public:
	OkButton(PreferencesWindow *pwindow);
	~OkButton();
	
	int handle_event();
	int keypress_event();
	
	PreferencesWindow *pwindow;
};

class PrefCancelButton : public BC_BigButton
{
public:
	PrefCancelButton(PreferencesWindow *pwindow);
	~PrefCancelButton();
	
	int handle_event();
	int keypress_event();
	
	PreferencesWindow *pwindow;
};

class PlaybackButton : public BC_BigButton
{
public:
	PlaybackButton(PreferencesWindow *pwindow);
	~PlaybackButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

class RecordButton : public BC_BigButton
{
public:
	RecordButton(PreferencesWindow *pwindow);
	~RecordButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

class IndexButton : public BC_BigButton
{
public:
	IndexButton(PreferencesWindow *pwindow);
	~IndexButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

class ViewButton : public BC_BigButton
{
public:
	ViewButton(PreferencesWindow *pwindow);
	~ViewButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

class PluginPrefsButton : public BC_BigButton
{
public:
	PluginPrefsButton(PreferencesWindow *pwindow);
	~PluginPrefsButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

class VideoPrefsButton : public BC_BigButton
{
public:
	VideoPrefsButton(PreferencesWindow *pwindow);
	~VideoPrefsButton();
	int handle_event();
	PreferencesWindow *pwindow;
};

#endif
