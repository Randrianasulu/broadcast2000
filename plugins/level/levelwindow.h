#ifndef LEVELWINDOW_H
#define LEVELWINDOW_H

#include "bcbase.h"

class LevelThread;
class LevelWindow;

#include "filehtal.h"
#include "mutex.h"
#include "level.h"

class LevelThread : public Thread
{
public:
	LevelThread(LevelMain *client);
	~LevelThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	LevelMain *client;
	LevelWindow *window;
};

class LevelSlider;

class LevelWindow : public BC_Window
{
public:
	LevelWindow(LevelMain *client);
	~LevelWindow();
	
	int create_objects();
	int close_event();
	
	LevelMain *client;
	LevelSlider *slider;
};

class LevelSlider : public BC_FSlider
{
public:
	LevelSlider(LevelMain *client);
	~LevelSlider();
	int handle_event();
	LevelMain *client;
};


#endif
