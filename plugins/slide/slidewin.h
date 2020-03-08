#ifndef SLIDEWIN_H
#define SLIDEWIN_H

#include "bcbase.h"

class SlideThread;
class SlideWin;

#include "filehtal.h"
#include "mutex.h"
#include "slide.h"

class SlideThread : public Thread
{
public:
	SlideThread(SlideMain *client);
	~SlideThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	SlideMain *client;
	SlideWin *window;
};

class SlideDirectionLeft;
class SlideDirectionRight;
class SlideReverse;

class SlideWin : public BC_Window
{
public:
	SlideWin(SlideMain *client);
	~SlideWin();

	int create_objects();
	int close_event();

	SlideMain *client;
	SlideDirectionLeft *left;
	SlideDirectionRight *right;
	SlideReverse *reverse;
};

class SlideDirectionLeft : public BC_Radial
{
public:
	SlideDirectionLeft(SlideWin *win, SlideMain *client, int x, int y);
	~SlideDirectionLeft();
	int handle_event();

	SlideMain *client;
	SlideWin *win;
};

class SlideDirectionRight : public BC_Radial
{
public:
	SlideDirectionRight(SlideWin *win, SlideMain *client, int x, int y);
	~SlideDirectionRight();
	int handle_event();

	SlideMain *client;
	SlideWin *win;
};

class SlideReverse : public BC_CheckBox
{
public:
	SlideReverse(SlideMain *client, int x, int y);
	~SlideReverse();
	int handle_event();

	SlideMain *client;
};


#endif
