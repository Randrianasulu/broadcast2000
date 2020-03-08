#ifndef DEINTERWINDOW_H
#define DEINTERWINDOW_H

#include "bcbase.h"

class DeInterlaceThread;
class DeInterlaceWindow;

#include "filehtal.h"
#include "mutex.h"
#include "deinterlace.h"

class DeInterlaceThread : public Thread
{
public:
	DeInterlaceThread(DeInterlaceMain *client);
	~DeInterlaceThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};

class DeInterlaceEven;
class DeInterlaceOdd;
class DeInterlaceAvg;
class DeInterlaceSwap;
class DeInterlaceSmart;

class DeInterlaceWindow : public BC_Window
{
public:
	DeInterlaceWindow(DeInterlaceMain *client);
	~DeInterlaceWindow();
	
	int create_objects();
	int close_event();
	int set_values(int even, int odd, int avg, int swap, int smart);
	
	DeInterlaceMain *client;
	DeInterlaceOdd *odd_fields;
	DeInterlaceEven *even_fields;
	DeInterlaceAvg *average_fields;
	DeInterlaceSwap *swap_fields;
	DeInterlaceSmart *smart_fields;
};

class DeInterlaceEven : public BC_CheckBox
{
public:
	DeInterlaceEven(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text);
	~DeInterlaceEven();
	int handle_event();

	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};

class DeInterlaceOdd : public BC_CheckBox
{
public:
	DeInterlaceOdd(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text);
	~DeInterlaceOdd();
	int handle_event();

	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};

class DeInterlaceAvg : public BC_CheckBox
{
public:
	DeInterlaceAvg(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text);
	~DeInterlaceAvg();
	int handle_event();

	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};

class DeInterlaceSwap : public BC_CheckBox
{
public:
	DeInterlaceSwap(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text);
	~DeInterlaceSwap();
	int handle_event();

	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};

class DeInterlaceSmart : public BC_CheckBox
{
public:
	DeInterlaceSmart(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text);
	~DeInterlaceSmart();
	int handle_event();

	DeInterlaceMain *client;
	DeInterlaceWindow *window;
};


#endif
