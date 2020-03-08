#ifndef BRIGHTWINDOW_H
#define BRIGHTWINDOW_H

#include "bcbase.h"

class BrightThread;
class BrightWindow;

#include "filehtal.h"
#include "mutex.h"
#include "brightness.h"

class BrightThread : public Thread
{
public:
	BrightThread(BrightnessMain *client);
	~BrightThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BrightnessMain *client;
	BrightWindow *window;
};

class BrightSlider;

class BrightWindow : public BC_Window
{
public:
	BrightWindow(BrightnessMain *client);
	~BrightWindow();
	
	int create_objects();
	int close_event();
	
	BrightnessMain *client;
	BrightSlider *bright_slider;
	BrightSlider *contrast_slider;
};

class BrightSlider : public BC_ISlider
{
public:
	BrightSlider(BrightnessMain *client, float *output, int x, int y);
	~BrightSlider();
	int handle_event();

	BrightnessMain *client;
	float *output;
};


#endif
