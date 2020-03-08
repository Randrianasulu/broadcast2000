#ifndef FLIPWINDOW_H
#define FLIPWINDOW_H

#include "bcbase.h"

class FlipThread;
class FlipWindow;

#include "filehtal.h"
#include "mutex.h"
#include "flip.h"

class FlipThread : public Thread
{
public:
	FlipThread(FlipMain *client);
	~FlipThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	FlipMain *client;
	FlipWindow *window;
};

class FlipToggle;

class FlipWindow : public BC_Window
{
public:
	FlipWindow(FlipMain *client);
	~FlipWindow();
	
	int create_objects();
	int close_event();
	
	FlipMain *client;
	FlipToggle *flip_vertical;
	FlipToggle *flip_horizontal;
};

class FlipToggle : public BC_CheckBox
{
public:
	FlipToggle(FlipMain *client, int *output, int x, int y);
	~FlipToggle();
	int handle_event();

	FlipMain *client;
	int *output;
};


#endif
