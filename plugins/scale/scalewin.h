#ifndef SCALEWIN_H
#define SCALEWIN_H

#include "bcbase.h"

class ScaleThread;
class ScaleWin;

#include "filehtal.h"
#include "mutex.h"
#include "scale.h"

class ScaleThread : public Thread
{
public:
	ScaleThread(ScaleMain *client);
	~ScaleThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	ScaleMain *client;
	ScaleWin *window;
};

class ScaleWidth;
class ScaleHeight;
class ScaleConstrain;

class ScaleWin : public BC_Window
{
public:
	ScaleWin(ScaleMain *client);
	~ScaleWin();

	int create_objects();
	int close_event();

	ScaleMain *client;
	ScaleWidth *width;
	ScaleHeight *height;
	ScaleConstrain *constrain;
};

class ScaleWidth : public BC_TextBox
{
public:
	ScaleWidth(ScaleWin *win, ScaleMain *client, int x, int y, char *string);
	~ScaleWidth();
	int handle_event();

	ScaleMain *client;
	ScaleWin *win;
};

class ScaleHeight : public BC_TextBox
{
public:
	ScaleHeight(ScaleWin *win, ScaleMain *client, int x, int y, char *string);
	~ScaleHeight();
	int handle_event();

	ScaleMain *client;
	ScaleWin *win;
};

class ScaleConstrain : public BC_CheckBox
{
public:
	ScaleConstrain(ScaleMain *client, int x, int y);
	~ScaleConstrain();
	int handle_event();

	ScaleMain *client;
};


#endif
