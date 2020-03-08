#ifndef TIMEWINDOW_H
#define TIMEWINDOW_H

#include "bcbase.h"
#include "timestretch.h"

class TimeWindowOK;
class TimeWindowCancel;
class TimeWindowWindow;
class TimeWindowAmount;

class TimeWindow : public BC_Window
{
public:
	TimeWindow();
	~TimeWindow();

	int create_objects(TimeStretch *plugin);

	TimeWindowOK *ok;
	TimeWindowCancel *cancel;
	TimeWindowAmount *percentage;
	TimeWindowWindow *window_size;
	TimeStretch *plugin;
};

class TimeWindowAmount : public BC_TextBox
{
public:
	TimeWindowAmount(TimeStretch *plugin, char *string);
	~TimeWindowAmount();
	
	int handle_event();
	TimeStretch *plugin;
};

class TimeWindowWindow : public BC_TextBox
{
public:
	TimeWindowWindow(TimeStretch *plugin, char *string);
	~TimeWindowWindow();
	
	int handle_event();
	TimeStretch *plugin;
};

class TimeWindowOK : public BC_BigButton
{
public:
	TimeWindowOK(TimeWindow *window);
	
	int handle_event();
	int keypress_event();
	
	TimeWindow *window;
};

class TimeWindowCancel : public BC_BigButton
{
public:
	TimeWindowCancel(TimeWindow *window);
	
	int handle_event();
	int keypress_event();
	
	TimeWindow *window;
};

#endif
