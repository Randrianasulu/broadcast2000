#ifndef STABILIZEWINDOW_H
#define STABILIZEWINDOW_H

#include "bcbase.h"

class StabilizeThread;
class StabilizeWindow;

#include "filehtal.h"
#include "mutex.h"
#include "stabilize.h"

class StabilizeThread : public Thread
{
public:
	StabilizeThread(StabilizeMain *client);
	~StabilizeThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	StabilizeMain *client;
	StabilizeWindow *window;
};

class StabilizeRange;
class StabilizeSize;
class StabilizeBound;
class StabilizeAccel;

class StabilizeWindow : public BC_Window
{
public:
	StabilizeWindow(StabilizeMain *client);
	~StabilizeWindow();
	
	int create_objects();
	int close_event();
	
	StabilizeMain *client;
	StabilizeRange *range;
	StabilizeSize *size;
	StabilizeAccel *accel;
};

class StabilizeRange : public BC_IPot
{
public:
	StabilizeRange(StabilizeMain *client, int x, int y);
	~StabilizeRange();
	int handle_event();

	StabilizeMain *client;
};

class StabilizeSize : public BC_IPot
{
public:
	StabilizeSize(StabilizeMain *client, int x, int y);
	~StabilizeSize();
	int handle_event();

	StabilizeMain *client;
};

class StabilizeAccel : public BC_IPot
{
public:
	StabilizeAccel(StabilizeMain *client, int x, int y);
	~StabilizeAccel();
	int handle_event();

	StabilizeMain *client;
};

#endif
