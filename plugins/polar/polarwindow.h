#ifndef POLARWINDOW_H
#define POLARWINDOW_H

#include "bcbase.h"

class PolarThread;
class PolarWindow;

#include "filehtal.h"
#include "mutex.h"
#include "polar.h"

class PolarThread : public Thread
{
public:
	PolarThread(PolarMain *client);
	~PolarThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	PolarMain *client;
	PolarWindow *window;
};

class DepthSlider;
class AngleSlider;
class AutomatedFn;

class PolarWindow : public BC_Window
{
public:
	PolarWindow(PolarMain *client);
	~PolarWindow();
	
	int create_objects();
	int close_event();
	
	PolarMain *client;
	DepthSlider *depth_slider;
	AngleSlider *angle_slider;
	AutomatedFn *automation[2];
};

class DepthSlider : public BC_ISlider
{
public:
	DepthSlider(PolarMain *client, int x, int y);
	~DepthSlider();
	int handle_event();

	PolarMain *client;
};

class AngleSlider : public BC_ISlider
{
public:
	AngleSlider(PolarMain *client, int x, int y);
	~AngleSlider();
	int handle_event();

	PolarMain *client;
};

class AutomatedFn : public BC_CheckBox
{
public:
	AutomatedFn(PolarMain *client, PolarWindow *window, int x, int y, int number);
	~AutomatedFn();
	int handle_event();

	PolarMain *client;
	PolarWindow *window;
	int number;
};


#endif
