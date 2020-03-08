#ifndef BANDWIPEWIN_H
#define BANDWIPEWIN_H

#include "bcbase.h"

class BandWipeThread;
class BandWipeWin;

#include "filehtal.h"
#include "mutex.h"
#include "bandwipe.h"

class BandWipeThread : public Thread
{
public:
	BandWipeThread(BandWipeMain *client);
	~BandWipeThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BandWipeMain *client;
	BandWipeWin *window;
};

class BandWipeTotal;
class BandWipeReverse;

class BandWipeWin : public BC_Window
{
public:
	BandWipeWin(BandWipeMain *client);
	~BandWipeWin();

	int create_objects();
	int close_event();

	BandWipeMain *client;
	BandWipeTotal *total;
	BandWipeReverse *reverse;
};

class BandWipeTotal : public BC_TextBox
{
public:
	BandWipeTotal(BandWipeMain *client, int x, int y);
	~BandWipeTotal();
	int handle_event();

	BandWipeMain *client;
};

class BandWipeReverse : public BC_CheckBox
{
public:
	BandWipeReverse(BandWipeMain *client, int x, int y);
	~BandWipeReverse();
	int handle_event();

	BandWipeMain *client;
};


#endif
