#ifndef SWAPWINDOW_H
#define SWAPWINDOW_H

#include "bcbase.h"

class SwapWindow;
class SwapMenu;
class SwapThread;

#include "filehtal.h"
#include "mutex.h"
#include "swapchannels.h"

class SwapThread : public Thread
{
public:
	SwapThread(SwapMain *client);
	~SwapThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	SwapMain *client;
	SwapWindow *window;
};
class SwapWindow : public BC_Window
{
public:
	SwapWindow(SwapMain *client);
	~SwapWindow();

	int create_objects();
	int close_event();

	SwapMain *client;
	SwapMenu *red;
	SwapMenu *green;
	SwapMenu *blue;
	SwapMenu *alpha;
};

class SwapMenu : public BC_PopupMenu
{
public:
	SwapMenu(SwapMain *client, 
			int *output, 
			int x, 
			int y);
	~SwapMenu();

	int handle_event();
	int add_items();

	SwapMain *client;
	int *output;
};

class SwapItem : public BC_PopupItem
{
public:
	SwapItem(SwapMenu *menu, char *title);
	~SwapItem();

	int handle_event();

	SwapMenu *menu;
	char *title;
};

#endif
