#ifndef BCPROGRESSBOX_H
#define BCPROGRESSBOX_H

class BC_ProgressWindow;
class BC_ProgressBox;

#include "bcbutton.h"
#include "bcprogress.inc"
#include "bctitle.inc"
#include "bcwindow.inc"
#include "thread.h"

class BC_ProgressBox : public Thread
{
public:
	BC_ProgressBox(const char *display, const char *text, long length, int cancel_button = 1);
	virtual ~BC_ProgressBox();
	
	void run();
	int update(long position);    // return 1 if cancelled
	int update_title(char *title);
	int update_length(long length);
	int cancelled();      // return 1 if cancelled
	int stop_progress();
	
	BC_ProgressWindow *pwindow;
	char *display;
	char *text;
	long length;
	int cancel_button;
};

class BC_ProgressWindowCancelButton;

class BC_ProgressWindow : public BC_Window
{
public:
	BC_ProgressWindow(const char *display = "", int cancel_button = 1);
	virtual ~BC_ProgressWindow();

	int create_objects(const char *text, long length);
	int update(long position);    // return 1 if cancelled

	const char *text;
	BC_ProgressBar *bar;
	BC_ProgressWindowCancelButton *cancel;
	BC_Title *caption;
	int cancelled;
	int cancel_button;
};

class BC_ProgressWindowCancelButton : public BC_BigButton
{
public:
	BC_ProgressWindowCancelButton(BC_ProgressWindow *pwindow);

	int handle_event();
	int keypress_event();

	BC_ProgressWindow *pwindow;
};

#endif
