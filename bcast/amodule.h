#ifndef AMODULE_H
#define AMODULE_H

class AModuleGUI;
class AModuleTitle;
class AModulePan;
class AModuleFade;
class AModuleInv;
class AModuleMute;
class AModuleReset;

#include "bcbase.h"
#include "amodule.inc"
#include "aplugin.inc"
#include "console.inc"
#include "datatype.h"
#include "filehtal.inc"
#include "floatautos.inc"
#include "maxchannels.h"
#include "module.h"
#include "sharedpluginlocation.inc"

class AModule : public Module
{
public:
	AModule() { };
	AModule(MainWindow *mwindow);
	virtual ~AModule();

	int create_objects(int pixel);

	int save(FileHTAL *htal);
	int load(FileHTAL *htal, int track_offset);

	int set_pixel(int pixel);
	int create_plugins(int &x, int &y);
	int flip_plugins(int &x, int &y);

	int init_meters(int total_peaks);
	int stop_meters();
	int reset_meter();

	int change_format();    // change between DB and INT for meter
	int flip_vertical(int pixel);
	int change_x(int new_pixel);
	int change_y(int new_pixel);
	int change_channels(int new_channels, int *value_positions);

// synchronization with tracks
	FloatAutos* get_pan_automation(int channel);  // get pan automation
	FloatAutos* get_fade_automation();       // get the fade automation for this module
	int set_title(char *text);
	int dump();
	int set_mute(int value);

// settings
	float pan[MAXCHANNELS];
	int pan_x, pan_y;
	int inv;
	int pixel;
	char string[32];       // for reading and writing pans

	DB db;


// meter
	int update_peak(int peak_number, float value);
	int update_meter(int peak_number, int last_peak, int total_peaks);
	float *peak_history;

	AModuleGUI *gui;
};

class AModuleGUI : public BC_SubWindow
{
public:
	AModuleGUI(MainWindow *mwindow, AModule *module, int x, int y, int w, int h);
	~AModuleGUI();

	int create_objects();     // don't want subwindow to call create_objects until pans are created
	int flip_vertical(int pixel);

	AModulePan* pan_stick;
	AModuleFade* fade_slider;
	AModuleTitle* title;
	AModuleInv *inv_toggle;
	AModuleMute *mute_toggle;
	BC_Meter *meter;
	AModuleReset *reset;
	MainWindow *mwindow;
	Console *console;
	AModule *module;
	BC_Title *inv_title;
	BC_Title *mute_title;
	BC_Title *fade_title;
};

class AModuleTitle : public BC_TextBox
{
public:
	AModuleTitle(AModule *module, Patch *patch, int x, int y);
	int handle_event();
	AModule *module;
	Patch *patch;
};

class AModuleInv : public BC_CheckBox
{
public:
	AModuleInv(Console *console, AModule *module, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();

	Console *console;
	AModule *module;
};

class AModuleMute : public BC_CheckBox
{
public:
	AModuleMute(Console *console, AModule *module, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();

	Console *console;
	AModule *module;
};

class AModulePan : public BC_Pan
{
public:
	AModulePan(AModule *module, int x, int y, int r, int virtual_r, float maxvalue, int total_values, int *value_positions, float *values);
	~AModulePan();

	int handle_event();
	int this_number;
	AModule *module;
};

class AModuleFade : public BC_FSlider
{
public:
	AModuleFade(AModule *module, int x, int y, int w, int h);
	~AModuleFade();

	int handle_event();
	AModule *module;
};

class AModuleReset : public BC_SmallButton
{
public:
	AModuleReset(AModule *module, int x, int y);
	~AModuleReset();

	int handle_event();
	AModule *module;
};

#endif
