#ifndef STATUSBAR_H
#define STATUSBAR_H

class FromTextBox;
class LengthTextBox;
class ToTextBox;

#include "bcbase.h"
#include "mainwindow.inc"
#include "mainwindowgui.inc"

class StatusBar : public BC_SubWindow
{
public:
	StatusBar(MainWindowGUI *gui);
	~StatusBar();

	int create_objects();
	int draw();
	int resize_event(int w, int h);
	int update();          // redraw the current values
	int update_playback(long new_position);       // update the playback position
	int set_selection(int which_one);

	MainWindowGUI *gui;
	BC_Title *zoom_value, *playback_value;
	LengthTextBox *length_value;
	FromTextBox *from_value;
	ToTextBox *to_value;
	MainWindow *mwindow;
	char string[256], string2[256];
	long old_position;
};



class FromTextBox : public BC_TextBox
{
public:
	FromTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y);
	int handle_event();
	int update_position(long new_position);
	char string[256], string2[256];
	MainWindow *mwindow;
	StatusBar *statusbar;
};


class LengthTextBox : public BC_TextBox
{
public:
	LengthTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y);
	int handle_event();
	int update_position(long new_position);
	char string[256], string2[256];
	MainWindow *mwindow;
	StatusBar *statusbar;
};

class ToTextBox : public BC_TextBox
{
public:
	ToTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y);
	int handle_event();
	int update_position(long new_position);
	char string[256], string2[256];
	MainWindow *mwindow;
	StatusBar *statusbar;
};




#endif
