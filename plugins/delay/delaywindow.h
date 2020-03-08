#ifndef DELAYWINDOW_H
#define DELAYWINDOW_H

#include "bcbase.h"

class DelayThread;
class DelayWindow;

#include "filehtal.h"
#include "mutex.h"
#include "delay.h"

class DelayThread : public Thread
{
public:
	DelayThread(DelayMain *client);
	~DelayThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	DelayMain *client;
	DelayWindow *window;
};

class DelayPot;

class DelayWindow : public BC_Window
{
public:
	DelayWindow(DelayMain *client);
	~DelayWindow();
	
	int create_objects();
	int close_event();
	
	DelayMain *client;
	DelayPot *slider;
};

class DelayPot : public BC_IPot
{
public:
	DelayPot(DelayMain *client);
	~DelayPot();
	int handle_event();
	DelayMain *client;
};


#endif
