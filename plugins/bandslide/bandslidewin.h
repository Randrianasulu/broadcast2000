#ifndef BANDSLIDEWIN_H
#define BANDSLIDEWIN_H

#include "bcbase.h"

class BandSlideThread;
class BandSlideWin;

#include "filehtal.h"
#include "mutex.h"
#include "bandslide.h"

class BandSlideThread : public Thread
{
public:
	BandSlideThread(BandSlideMain *client);
	~BandSlideThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BandSlideMain *client;
	BandSlideWin *window;
};

class BandSlideTotal;
class BandSlideReverse;

class BandSlideWin : public BC_Window
{
public:
	BandSlideWin(BandSlideMain *client);
	~BandSlideWin();

	int create_objects();
	int close_event();

	BandSlideMain *client;
	BandSlideTotal *total;
	BandSlideReverse *reverse;
};

class BandSlideTotal : public BC_TextBox
{
public:
	BandSlideTotal(BandSlideMain *client, int x, int y);
	~BandSlideTotal();
	int handle_event();

	BandSlideMain *client;
};

class BandSlideReverse : public BC_CheckBox
{
public:
	BandSlideReverse(BandSlideMain *client, int x, int y);
	~BandSlideReverse();
	int handle_event();

	BandSlideMain *client;
};


#endif
