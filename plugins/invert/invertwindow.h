#ifndef LEVELWINDOW_H
#define LEVELWINDOW_H

#include "bcbase.h"

class InvertThread;
class InvertWindow;

#include "filehtal.h"
#include "mutex.h"
#include "invert.h"

class InvertThread : public Thread
{
public:
	InvertThread(InvertMain *client);
	~InvertThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	InvertMain *client;
	InvertWindow *window;
};

class InvertToggle;

class InvertWindow : public BC_Window
{
public:
	InvertWindow(InvertMain *client);
	~InvertWindow();

	int create_objects();
	int close_event();

	InvertMain *client;
	InvertToggle *toggle;
};

class InvertToggle : public BC_CheckBox
{
public:
	InvertToggle(InvertMain *client);
	~InvertToggle();
	int handle_event();
	InvertMain *client;
};


#endif
