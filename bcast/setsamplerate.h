#ifndef SETSAMPLERATE_H
#define SETSAMPLERATE_H

#include "bcbase.h"
#include "mainwindow.inc"
#include "thread.h"

class SetSampleRate : public BC_MenuItem, public Thread
{
public:
	SetSampleRate(MainWindow *mwindow);
	int handle_event();
	void run();
	
	MainWindow *mwindow;
};

class SetSampleRateOkButton;
class SetSampleRateCancelButton;
class SetSampleRateTextBox;

class SetSampleRateWindow : public BC_Window
{
public:
	SetSampleRateWindow(char *display = "");
	~SetSampleRateWindow();
	int create_objects(MainWindow *mwindow);
	
	MainWindow *mwindow;
	long sample_rate;
	SetSampleRateOkButton *ok;
	SetSampleRateCancelButton *cancel;
	SetSampleRateTextBox *text;
};

class SetSampleRateOkButton : public BC_BigButton
{
public:
	SetSampleRateOkButton(SetSampleRateWindow *srwindow);
	
	int handle_event();
	int keypress_event();
	
	SetSampleRateWindow *srwindow;
	MainWindow *mwindow;
};

class SetSampleRateCancelButton : public BC_BigButton
{
public:
	SetSampleRateCancelButton(SetSampleRateWindow *srwindow);
	
	int handle_event();
	int keypress_event();
	
	SetSampleRateWindow *srwindow;
};

class SetSampleRateTextBox : public BC_TextBox
{
public:
	SetSampleRateTextBox(SetSampleRateWindow *srwindow, char *text);
	
	int handle_event();
	
	SetSampleRateWindow *srwindow;
	MainWindow *mwindow;
};

#endif

