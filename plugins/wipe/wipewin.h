#ifndef WIPEWIN_H
#define WIPEWIN_H

#include "bcbase.h"

class WipeThread;
class WipeWin;

#include "filehtal.h"
#include "mutex.h"
#include "wipe.h"

class WipeThread : public Thread
{
public:
	WipeThread(WipeMain *client);
	~WipeThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	WipeMain *client;
	WipeWin *window;
};

class WipeDirectionLeft;
class WipeDirectionRight;
class WipeReverse;

class WipeWin : public BC_Window
{
public:
	WipeWin(WipeMain *client);
	~WipeWin();

	int create_objects();
	int close_event();

	WipeMain *client;
	WipeDirectionLeft *left;
	WipeDirectionRight *right;
	WipeReverse *reverse;
};

class WipeDirectionLeft : public BC_Radial
{
public:
	WipeDirectionLeft(WipeWin *win, WipeMain *client, int x, int y);
	~WipeDirectionLeft();
	int handle_event();

	WipeMain *client;
	WipeWin *win;
};

class WipeDirectionRight : public BC_Radial
{
public:
	WipeDirectionRight(WipeWin *win, WipeMain *client, int x, int y);
	~WipeDirectionRight();
	int handle_event();

	WipeMain *client;
	WipeWin *win;
};

class WipeReverse : public BC_CheckBox
{
public:
	WipeReverse(WipeMain *client, int x, int y);
	~WipeReverse();
	int handle_event();

	WipeMain *client;
};


#endif
