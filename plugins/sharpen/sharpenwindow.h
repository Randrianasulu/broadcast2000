#ifndef SHARPENWINDOW_H
#define SHARPENWINDOW_H

#include "bcbase.h"

class SharpenThread;
class SharpenWindow;
class SharpenInterlace;

#include "filehtal.h"
#include "mutex.h"
#include "sharpen.h"

class SharpenThread : public Thread
{
public:
	SharpenThread(SharpenMain *client);
	~SharpenThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	SharpenMain *client;
	SharpenWindow *window;
};

class SharpenSlider;

class SharpenWindow : public BC_Window
{
public:
	SharpenWindow(SharpenMain *client);
	~SharpenWindow();
	
	int create_objects();
	int close_event();
	
	SharpenMain *client;
	SharpenSlider *sharpen_slider;
	SharpenInterlace *sharpen_interlace;
};

class SharpenSlider : public BC_ISlider
{
public:
	SharpenSlider(SharpenMain *client, float *output, int x, int y);
	~SharpenSlider();
	int handle_event();

	SharpenMain *client;
	float *output;
};

class SharpenInterlace : public BC_CheckBox
{
public:
	SharpenInterlace(SharpenMain *client, int x, int y);
	~SharpenInterlace();
	int handle_event();

	SharpenMain *client;
};


#endif
