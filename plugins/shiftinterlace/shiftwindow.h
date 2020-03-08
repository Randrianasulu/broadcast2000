#ifndef DEINTERWINDOW_H
#define DEINTERWINDOW_H

#include "bcbase.h"

class ShiftThread;
class ShiftWindow;

#include "filehtal.h"
#include "mutex.h"
#include "shiftinterlace.h"

class ShiftThread : public Thread
{
public:
	ShiftThread(ShiftInterlaceMain *client);
	~ShiftThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	ShiftInterlaceMain *client;
	ShiftWindow *window;
};

class ShiftOffset;

class ShiftWindow : public BC_Window
{
public:
	ShiftWindow(ShiftInterlaceMain *client);
	~ShiftWindow();
	
	int create_objects();
	int close_event();
	int set_values(int even, int odd, int avg, int swap);
	
	ShiftInterlaceMain *client;
	ShiftOffset *odd_offset, *even_offset;
};

class ShiftOffset : public BC_ISlider
{
public:
	ShiftOffset(ShiftInterlaceMain *client, int *output, int x, int y);
	~ShiftOffset();
	int handle_event();

	ShiftInterlaceMain *client;
	int *output;
};


#endif
