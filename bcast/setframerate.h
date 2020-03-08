#ifndef SETFRAMERATE_H
#define SETFRAMERATE_H

#include "bcbase.h"
#include "setframerate.inc"
#include "thread.h"

class SetFrameRate : public BC_MenuItem
{
public:
	SetFrameRate(MainWindow *mwindow);
	~SetFrameRate();

	int handle_event();
	
	MainWindow *mwindow;
	SetFrameRateThread *thread;
};

class SetFrameRateThread : public Thread
{
public:
	SetFrameRateThread(MainWindow *mwindow);
	~SetFrameRateThread();

	void run();

	MainWindow *mwindow;
	float frame_rate;
	int scale_data;
};

class SetFrameRateWindow : public BC_Window
{
public:
	SetFrameRateWindow(SetFrameRateThread *thread);
	~SetFrameRateWindow();
	int create_objects();

	SetFrameRateThread *thread;
};

class SetFrameRateOkButton : public BC_BigButton
{
public:
	SetFrameRateOkButton(SetFrameRateWindow *frwindow);

	int handle_event();
	int keypress_event();

	SetFrameRateWindow *frwindow;
};

class SetFrameRateCancelButton : public BC_BigButton
{
public:
	SetFrameRateCancelButton(SetFrameRateWindow *frwindow);
	
	int handle_event();
	int keypress_event();
	
	SetFrameRateWindow *frwindow;
};

class SetFrameRateTextBox : public BC_TextBox
{
public:
	SetFrameRateTextBox(SetFrameRateThread *thread);
	int handle_event();
	SetFrameRateThread *thread;
};

class SetFrameRateMoveData : public BC_CheckBox
{
public:
	SetFrameRateMoveData(SetFrameRateThread *thread);
	int handle_event();
	SetFrameRateThread *thread;
};

#endif

