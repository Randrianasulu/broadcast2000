#ifndef SRATEWINDOW_H
#define SRATEWINDOW_H

#include "bcbase.h"

class NormalizeWindowOK;
class NormalizeWindowCancel;
class NormalizeWindowOverload;
class NormalizeWindowSeparate;

class NormalizeWindow : public BC_Window
{
public:
	NormalizeWindow();
	~NormalizeWindow();
	
	int create_objects(float *db_over, int *seperate_tracks);
	
	float *db_over;
	int *separate_tracks;
	NormalizeWindowOK *ok;
	NormalizeWindowCancel *cancel;
	NormalizeWindowOverload *overload_text;
	NormalizeWindowSeparate *separate_tracks_toggle;
};

class NormalizeWindowOverload : public BC_TextBox
{
public:
	NormalizeWindowOverload(float *db_over);
	~NormalizeWindowOverload();
	
	int handle_event();
	float *db_over;
};

class NormalizeWindowSeparate : public BC_CheckBox
{
public:
	NormalizeWindowSeparate(int *separate_tracks);
	~NormalizeWindowSeparate();
	
	int handle_event();
	int *separate_tracks;
};

class NormalizeWindowOK : public BC_BigButton
{
public:
	NormalizeWindowOK(NormalizeWindow *window);
	
	int handle_event();
	int keypress_event();
	
	NormalizeWindow *window;
};

class NormalizeWindowCancel : public BC_BigButton
{
public:
	NormalizeWindowCancel(NormalizeWindow *window);
	
	int handle_event();
	int keypress_event();
	
	NormalizeWindow *window;
};

#endif
