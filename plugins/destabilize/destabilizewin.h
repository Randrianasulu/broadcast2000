#ifndef DESTABILIZEWIN_H
#define DESTABILIZEWIN_H

#include "bcbase.h"

class DestabilizeThread;
class DestabilizeWindow;

#include "filehtal.h"
#include "mutex.h"
#include "destabilize.h"

class DestabilizeThread : public Thread
{
public:
	DestabilizeThread(DestabilizeMain *client);
	~DestabilizeThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	DestabilizeMain *client;
	DestabilizeWindow *window;
};

class DestabilizeRange;
class DestabilizeAccel;
class DestabilizeSpeed;

class DestabilizeWindow : public BC_Window
{
public:
	DestabilizeWindow(DestabilizeMain *client);
	~DestabilizeWindow();
	
	int create_objects();
	int close_event();
	
	DestabilizeMain *client;
	DestabilizeRange *range;
	DestabilizeAccel *accel;
	DestabilizeSpeed *speed;
};

class DestabilizeRange : public BC_IPot
{
public:
	DestabilizeRange(DestabilizeMain *client, int x, int y);
	~DestabilizeRange();
	int handle_event();

	DestabilizeMain *client;
};

class DestabilizeAccel : public BC_IPot
{
public:
	DestabilizeAccel(DestabilizeMain *client, int x, int y);
	~DestabilizeAccel();
	int handle_event();

	DestabilizeMain *client;
};

class DestabilizeSpeed : public BC_IPot
{
public:
	DestabilizeSpeed(DestabilizeMain *client, int x, int y);
	~DestabilizeSpeed();
	int handle_event();

	DestabilizeMain *client;
};

#endif
