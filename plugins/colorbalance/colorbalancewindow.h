#ifndef COLORBALANCEWINDOW_H
#define COLORBALANCEWINDOW_H

#include "bcbase.h"

class ColorBalanceThread;
class ColorBalanceWindow;

class ColorBalanceSlider;
class ColorBalancePreserve;
class ColorBalanceLock;

#include "filehtal.h"
#include "mutex.h"
#include "colorbalance.h"

class ColorBalanceThread : public Thread
{
public:
	ColorBalanceThread(ColorBalanceMain *client);
	~ColorBalanceThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	ColorBalanceMain *client;
	ColorBalanceWindow *window;
};

class ColorBalanceWindow : public BC_Window
{
public:
	ColorBalanceWindow(ColorBalanceMain *client);
	~ColorBalanceWindow();

	int create_objects();
	int close_event();

	ColorBalanceMain *client;
	ColorBalanceSlider *cyan;
	ColorBalanceSlider *magenta;
	ColorBalanceSlider *yellow;
    ColorBalanceLock *lock_params;
    ColorBalancePreserve *preserve;
};

class ColorBalanceSlider : public BC_ISlider
{
public:
	ColorBalanceSlider(ColorBalanceMain *client, float *output, int x, int y);
	~ColorBalanceSlider();
	int handle_event();

	ColorBalanceMain *client;
	float *output;
    float old_value;
};

class ColorBalancePreserve : public BC_CheckBox
{
public:
	ColorBalancePreserve(ColorBalanceMain *client, int x, int y);
    ~ColorBalancePreserve();
    
    int handle_event();
    ColorBalanceMain *client;
};

class ColorBalanceLock : public BC_CheckBox
{
public:
	ColorBalanceLock(ColorBalanceMain *client, int x, int y);
    ~ColorBalanceLock();
    
    int handle_event();
    ColorBalanceMain *client;
};

#endif
