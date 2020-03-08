#ifndef RGB601WINDOW_H
#define RGB601WINDOW_H

#include "bcbase.h"

class RGB601Thread;
class RGB601Window;

#include "filehtal.h"
#include "mutex.h"
#include "rgb601.h"

class RGB601Thread : public Thread
{
public:
	RGB601Thread(RGB601Main *client);
	~RGB601Thread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	RGB601Main *client;
	RGB601Window *window;
};

class RGBto601Toggle;
class _601toRGBToggle;

class RGB601Window : public BC_Window
{
public:
	RGB601Window(RGB601Main *client);
	~RGB601Window();
	
	int create_objects();
	int close_event();
	
	RGB601Main *client;
	RGBto601Toggle *rgb_to_601;
	_601toRGBToggle *_601_to_rgb;
};

class RGBto601Toggle : public BC_CheckBox
{
public:
	RGBto601Toggle(RGB601Main *client, RGB601Window *window, int x, int y);
	~RGBto601Toggle();
	int handle_event();

	RGB601Main *client;
	RGB601Window *window;
};

class _601toRGBToggle : public BC_CheckBox
{
public:
	_601toRGBToggle(RGB601Main *client, RGB601Window *window, int x, int y);
	~_601toRGBToggle();
	int handle_event();

	RGB601Main *client;
	RGB601Window *window;
};


#endif
