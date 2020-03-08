#ifndef PROGRESSBOX_H
#define PROGRESSBOX_H

#include "bcbase.h"
#include "progressbox.inc"
#include "thread.h"

class ProgressBox : public Thread
{
public:
	ProgressBox(char *display, char *text, long length, int cancel_button = 1);
	~ProgressBox();

	void run();
	int update(long position);    // return 1 if cancelled
	int update_title(char *new_title);
	int update_length(long new_length);
	int cancelled();      // return 1 if cancelled
	int stop_progress();

	ProgressWindow *pwindow;
	char *display;
	char *text;
	long length;
	int cancel_button;
};

class ProgressWindowCancelButton;

class ProgressWindow : public BC_Window
{
public:
	ProgressWindow(char *display = "", int cancel_button = 1);
	~ProgressWindow();

	int create_objects(char *text, long length);
	int update(long position);    // return 1 if cancelled

	char *text;
	BC_ProgressBar *bar;
	BC_Title *title;
	ProgressWindowCancelButton *cancel;
	int cancelled;
	int cancel_button;
};

class ProgressWindowCancelButton : public BC_BigButton
{
public:
	ProgressWindowCancelButton(ProgressWindow *pwindow);

	int handle_event();
	int keypress_event();

	ProgressWindow *pwindow;
};

#endif
