#ifndef MENUEFFECTS_H
#define MENUEFFECTS_H

#include "assets.inc"
#include "bitspopup.h"
#include "browsebutton.h"
#include "compresspopup.h"
#include "formatpopup.h"
#include "formattools.h"
#include "mainmenu.inc"
#include "mainwindow.inc"
#include "pluginarray.inc"
#include "pluginserver.inc"
#include "thread.h"

class MenuEffectThread;
class MenuEffects : public BC_MenuItem
{
public:
	MenuEffects(MainWindow *mwindow);
	virtual ~MenuEffects();

	int handle_event();

	MainWindow *mwindow;
	MenuEffectThread *thread;
};

class MenuEffectThread : public Thread
{
public:
	MenuEffectThread(MainWindow *mwindow);
	virtual ~MenuEffectThread();

	void run();
	int set_title(char *text);  // set the effect to be run by a menuitem
	virtual int get_recordable_tracks(Asset *asset) { return 0; };
	virtual int get_derived_attributes(Asset *asset, Defaults *defaults) { return 0; };
	virtual int save_derived_attributes(Asset *asset, Defaults *defaults) { return 0; };
	virtual PluginArray* create_plugin_array(MainWindow *mwindow) { return 0; };
	virtual int convert_units(long &start, long &end) { return 0; };
	virtual int fix_menu(char *title) { return 0; };

	MainWindow *mwindow;
	char title[1024];
	int dither, to_tracks, realtime;
};


class MenuEffectItem : public BC_MenuItem
{
public:
	MenuEffectItem(MenuEffects *menueffect, char *string);
	virtual ~MenuEffectItem() {  };
	int handle_event();
	MenuEffects *menueffect;
};






class MenuEffectWindowOK;
class MenuEffectWindowCancel;
class MenuEffectWindowList;
class MenuEffectWindowToTracks;


class MenuEffectWindow : public BC_Window
{
public:
	MenuEffectWindow(MenuEffectThread *menueffects, ArrayList<BC_ListBoxItem*> *plugin_list, Asset *asset);
	virtual ~MenuEffectWindow();
	
	int create_objects();

	MenuEffectThread *menueffects;
	ArrayList<BC_ListBoxItem*> *plugin_list;
	Asset *asset;

	int result;
	FormatTools *format_tools;
	MenuEffectWindowOK *ok;
	MenuEffectWindowCancel *cancel;
	MenuEffectWindowList *list;
	MenuEffectWindowToTracks *to_tracks_button;
};

class MenuEffectWindowOK : public BC_BigButton
{
public:
	MenuEffectWindowOK(MenuEffectWindow *window);
	
	int handle_event();
	int keypress_event();
	
	MenuEffectWindow *window;
};

class MenuEffectWindowCancel : public BC_BigButton
{
public:
	MenuEffectWindowCancel(MenuEffectWindow *window);

	int handle_event();
	int keypress_event();

	MenuEffectWindow *window;
};

class MenuEffectWindowList : public BC_ListBox
{
public:
	MenuEffectWindowList(MenuEffectWindow *window, int x, ArrayList<BC_ListBoxItem*> *plugin_list);

	int handle_event();
	MenuEffectWindow *window;
};


class MenuEffectWindowToTracks : public BC_CheckBox
{
public:
	MenuEffectWindowToTracks(MenuEffectWindow *window, int x, int default_);
	~MenuEffectWindowToTracks();

	int handle_event();
	MenuEffectWindow *window;
};

class MenuEffectPromptOK;
class MenuEffectPromptCancel;


class MenuEffectPrompt : public BC_Window
{
public:
	MenuEffectPrompt();
	~MenuEffectPrompt();
	
	int create_objects();

	MenuEffectPromptOK *ok;
	MenuEffectPromptCancel *cancel;
};


class MenuEffectPromptOK : public BC_BigButton
{
public:
	MenuEffectPromptOK(MenuEffectPrompt *window);
	~MenuEffectPromptOK();
	int handle_event();
	int keypress_event();
	MenuEffectPrompt *window;
};

class MenuEffectPromptCancel : public BC_BigButton
{
public:
	MenuEffectPromptCancel(MenuEffectPrompt *window);
	~MenuEffectPromptCancel();
	int handle_event();
	int keypress_event();
	MenuEffectPrompt *window;
};


#endif
