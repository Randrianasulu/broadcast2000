#ifndef BLURWINDOW_H
#define BLURWINDOW_H

#include "bcbase.h"

class BlurThread;
class BlurWindow;

#include "filehtal.h"
#include "mutex.h"
#include "blur.h"

class BlurThread : public Thread
{
public:
	BlurThread(BlurMain *client);
	~BlurThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BlurMain *client;
	BlurWindow *window;
};

class BlurVertical;
class BlurHorizontal;
class BlurRadius;

class BlurWindow : public BC_Window
{
public:
	BlurWindow(BlurMain *client);
	~BlurWindow();
	
	int create_objects();
	int close_event();
	
	BlurMain *client;
	BlurVertical *vertical;
	BlurHorizontal *horizontal;
	BlurRadius *radius;
};

class BlurRadius : public BC_IPot
{
public:
	BlurRadius(BlurMain *client, int x, int y);
	~BlurRadius();
	int handle_event();

	BlurMain *client;
};

class BlurVertical : public BC_CheckBox
{
public:
	BlurVertical(BlurMain *client, BlurWindow *window, int x, int y);
	~BlurVertical();
	int handle_event();

	BlurMain *client;
	BlurWindow *window;
};

class BlurHorizontal : public BC_CheckBox
{
public:
	BlurHorizontal(BlurMain *client, BlurWindow *window, int x, int y);
	~BlurHorizontal();
	int handle_event();

	BlurMain *client;
	BlurWindow *window;
};


#endif
