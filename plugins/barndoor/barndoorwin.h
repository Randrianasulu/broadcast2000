#ifndef BARNDOORWIN_H
#define BARNDOORWIN_H

#include "bcbase.h"

class BarnDoorThread;
class BarnDoorWin;

#include "filehtal.h"
#include "mutex.h"
#include "barndoor.h"

class BarnDoorThread : public Thread
{
public:
	BarnDoorThread(BarnDoorMain *client);
	~BarnDoorThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BarnDoorMain *client;
	BarnDoorWin *window;
};

class BarnDoorReverse;

class BarnDoorWin : public BC_Window
{
public:
	BarnDoorWin(BarnDoorMain *client);
	~BarnDoorWin();

	int create_objects();
	int close_event();

	BarnDoorMain *client;
	BarnDoorReverse *reverse;
};

class BarnDoorReverse : public BC_CheckBox
{
public:
	BarnDoorReverse(BarnDoorMain *client, int x, int y);
	~BarnDoorReverse();
	int handle_event();

	BarnDoorMain *client;
};


#endif
