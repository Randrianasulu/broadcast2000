#ifndef PITCHWINDOW_H
#define PITCHWINDOW_H

class PitchWindow;
class PitchThread;

#include "bcbase.h"
#include "pitch.h"
#include "mutex.h"

class PitchThread : public Thread
{
public:
	PitchThread(Pitch *plugin);
	~PitchThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Pitch *plugin;
	PitchWindow *window;
};

class PitchAmount;
class PitchWindowSize;


class PitchWindow : public BC_Window
{
public:
	PitchWindow(Pitch *plugin);
	~PitchWindow();
	
	int create_objects();
	int close_event();
	int update_gui();

	Pitch *plugin;
	PitchAmount *freq_offset;
	PitchWindowSize *windowsize;
};

class PitchAmount : public BC_IPot
{
public:
	PitchAmount(Pitch *plugin, int x, int y);
	~PitchAmount();
	int handle_event();
	Pitch *plugin;
};

class PitchWindowSize : public BC_TextBox
{
public:
	PitchWindowSize(Pitch *plugin, int x, int y);
	~PitchWindowSize();
	int handle_event();
	Pitch *plugin;
};


#endif
