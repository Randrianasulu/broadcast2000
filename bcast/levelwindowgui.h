#ifndef LEVELWINDOWGUI_H
#define LEVELWINDOWGUI_H

class LevelWindowReset;

#include "bcbase.h"
#include "levelwindow.inc"
#include "maxchannels.h"

class LevelWindowGUI : public BC_Window
{
public:
	LevelWindowGUI(LevelWindow *thread, int w, int h);
	~LevelWindowGUI();

	int create_objects();
	int resize_event(int w, int h);
	int new_meter();
	int delete_meter();
	int close_event();
	int draw_db_scale();
	int delete_db_scale();
	int reset_over();
	int keypress_event();

	int total_meters;
	BC_Meter *meters[MAXCHANNELS];
	BC_Title *titles[MAXCHANNELS];
	BC_Title *db_title[6];
	LevelWindowReset *reset_button;

	LevelWindow *thread;
	int METER_H;
	int db_scale;
};

class LevelWindowReset : public BC_SmallButton
{
public:
	LevelWindowReset(LevelWindowGUI *gui, int x, int y);
	~LevelWindowReset();
	int handle_event();
	LevelWindowGUI *gui;
};


#endif
