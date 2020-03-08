#ifndef WIPEWIN_H
#define WIPEWIN_H

#include "bcbase.h"

class IrisSquareThread;
class IrisSquareWin;

#include "filehtal.h"
#include "mutex.h"
#include "irissquare.h"

class IrisSquareThread : public Thread
{
public:
	IrisSquareThread(IrisSquareMain *client);
	~IrisSquareThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	IrisSquareMain *client;
	IrisSquareWin *window;
};

class IrisSquareReverse;

class IrisSquareWin : public BC_Window
{
public:
	IrisSquareWin(IrisSquareMain *client);
	~IrisSquareWin();

	int create_objects();
	int close_event();

	IrisSquareMain *client;
	IrisSquareReverse *reverse;
};

class IrisSquareReverse : public BC_CheckBox
{
public:
	IrisSquareReverse(IrisSquareMain *client, int x, int y);
	~IrisSquareReverse();
	int handle_event();

	IrisSquareMain *client;
};


#endif
