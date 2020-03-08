#ifndef VMODULE_H
#define VMODULE_H

class VModuleGUI;
class VModuleTitle;
class VModuleFade;
class VModuleMute;
class VModuleMode;

#define VMODULEHEIGHT 91
#define VMODULEWIDTH 106


#include "bcbase.h"
#include "datatype.h"
#include "filehtal.inc"
#include "floatautos.inc"
#include "maxchannels.h"
#include "module.h"
#include "sharedpluginlocation.inc"

class VModule : public Module
{
public:
	VModule() { };
	VModule(MainWindow *mwindow);
	virtual ~VModule();

	int create_objects(int pixel);

	int save(FileHTAL *htal);
	int load(FileHTAL *htal, int track_offset);

	int set_pixel(int pixel);
	int create_plugins(int &x, int &y);
	int flip_plugins(int &x, int &y);

	int flip_vertical(int pixel);
	int change_x(int new_pixel);
	int change_y(int new_pixel);
	int set_mute(int value);

// synchronization with tracks
	FloatAutos* get_fade_automation();       // get the fade automation for this module
	int set_title(char *text);

// Method of alpha channel calculation
	int mode;
	int pixel;

	VModuleGUI *gui;
};

class VModuleGUI : public BC_SubWindow
{
public:
	VModuleGUI(MainWindow *mwindow, VModule *module, int x, int y, int w, int h);
	~VModuleGUI();
	
	int create_objects();     // don't want subwindow to call create_objects until pans are created
	int flip_vertical(int pixel);

	VModuleMute* mute_toggle;
	VModuleFade* fade_slider;
	VModuleTitle* title;
	MainWindow *mwindow;
	Console *console;
	VModule *module;
	BC_Title *fade_title;
	BC_Title *mute_title;
	VModuleMode *mode_popup;
	BC_Title *mode_title;
};

class VModuleTitle : public BC_TextBox
{
public:
	VModuleTitle(VModule *module, Patch *patch, int x, int y);
	int handle_event();
	VModule *module;
	Patch *patch;
};

class VModuleFade : public BC_ISlider
{
public:
	VModuleFade(VModule *module, int x, int y, int w, int h);
	~VModuleFade();
	
	int handle_event();
	VModule *module;
};

class VModuleMute : public BC_CheckBox
{
public:
	VModuleMute(Console *console, VModule *module, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();

	Console *console;
	VModule *module;
};

class VModuleMode : public BC_PopupMenu
{
public:
	VModuleMode(Console *console, VModule *module, int x, int y);
	~VModuleMode();

	int handle_event();
	int add_items();         // add initial items
	char* mode_to_text(int mode);

	Console *console;
	VModule *module;
};

class VModuleModeItem : public BC_PopupItem
{
public:
	VModuleModeItem(VModuleMode *popup, char *text, int mode);
	~VModuleModeItem();

	int handle_event();
	VModuleMode *popup;
	int mode;
};

#endif
