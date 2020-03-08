#ifndef CONVOLVEWINDOW_H
#define CONVOLVEWINDOW_H

class ConvolveWindow;
class ConvolveThread;

#include "bcbase.h"
#include "convolve.h"
#include "mutex.h"

class ConvolveThread : public Thread
{
public:
	ConvolveThread(Convolve *plugin);
	~ConvolveThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Convolve *plugin;
	ConvolveWindow *window;
};

class ConvolveAmount;
class ConvolveWindowSize;
class ConvolveAutomate;


class ConvolveWindow : public BC_Window
{
public:
	ConvolveWindow(Convolve *plugin);
	~ConvolveWindow();
	
	int create_objects();
	int close_event();
	int update_gui();

	Convolve *plugin;
	ConvolveAmount *chan_level[2];
	ConvolveWindowSize *windowsize;
	ConvolveAutomate *automate_level[2];
};

class ConvolveAmount : public BC_FPot
{
public:
	ConvolveAmount(Convolve *plugin, int x, int y, int number);
	~ConvolveAmount();
	int handle_event();
	Convolve *plugin;
	int number;
};

class ConvolveWindowSize : public BC_TextBox
{
public:
	ConvolveWindowSize(Convolve *plugin, int x, int y);
	~ConvolveWindowSize();
	int handle_event();
	Convolve *plugin;
};

class ConvolveAutomate : public BC_CheckBox
{
public:
	ConvolveAutomate(Convolve *plugin, ConvolveWindow *window, int number, int x, int y);
	~ConvolveAutomate();
	
	int handle_event();
	Convolve *plugin;
	ConvolveWindow *window;
	int number;
};
#endif
