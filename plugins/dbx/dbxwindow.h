#ifndef DBXWINDOW_H
#define DBXWINDOW_H

#include "guicast.h"

class DBXThread;
class DBXWindow;

#include "filehtal.h"
#include "mutex.h"
#include "dbx.h"

class DBXThread : public Thread
{
public:
	DBXThread(DBXMain *client);
	~DBXThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	DBXMain *client;
	DBXWindow *window;
};

class DBXGain;
class DBXWindowPot;

class DBXWindow : public BC_Window
{
public:
	DBXWindow(DBXMain *client);
	~DBXWindow();

	int create_objects();
	int close_event();

	DBXMain *client;
	DBXGain *gain;
	DBXWindowPot *window;
};

class DBXGain : public BC_FPot
{
public:
	DBXGain(DBXMain *plugin, int x, int y);
	int handle_event();
	DBXMain *plugin;
};

class DBXWindowPot : public BC_IPot
{
public:
	DBXWindowPot(DBXMain *plugin, int x, int y);
	int handle_event();
	DBXMain *plugin;
};



#endif
