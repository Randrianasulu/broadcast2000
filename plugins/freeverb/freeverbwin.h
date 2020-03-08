#ifndef FREEVERBWINDOW_H
#define FREEVERBWINDOW_H

#include "guicast.h"

class FreeverbThread;
class FreeverbWindow;

#include "freeverb.h"
#include "mutex.h"
#include "thread.h"

class FreeverbThread : public Thread
{
public:
	FreeverbThread(Freeverb *freeverb);
	~FreeverbThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Freeverb *freeverb;
	FreeverbWindow *window;
};

class FreeverbRoomsize;
class FreeverbDamping;
class FreeverbWetness;
class FreeverbDryness;
class FreeverbGain;

class FreeverbWindow : public BC_Window
{
public:
	FreeverbWindow(Freeverb *freeverb);
	~FreeverbWindow();

	int create_objects();
	int close_event();

	Freeverb *freeverb;
	FreeverbRoomsize *roomsize;
	FreeverbDamping *damping;
	FreeverbWetness *wetness;
	FreeverbDryness *dryness;
	FreeverbGain *gain;
};

class FreeverbRoomsize : public BC_PercentagePot
{
public:
	FreeverbRoomsize(Freeverb *freeverb, int x, int y);
	int handle_event();
	Freeverb *freeverb;
};

class FreeverbDamping : public BC_FPot
{
public:
	FreeverbDamping(Freeverb *freeverb, int x, int y);
	int handle_event();
	Freeverb *freeverb;
};

class FreeverbWetness : public BC_FPot
{
public:
	FreeverbWetness(Freeverb *freeverb, int x, int y);
	int handle_event();
	Freeverb *freeverb;
};

class FreeverbDryness : public BC_FPot
{
public:
	FreeverbDryness(Freeverb *freeverb, int x, int y);
	int handle_event();
	Freeverb *freeverb;
};

class FreeverbGain : public BC_FPot
{
public:
	FreeverbGain(Freeverb *freeverb, int x, int y);
	int handle_event();
	Freeverb *freeverb;
};





#endif
