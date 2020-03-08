#ifndef SPECTROGRAMWINDOW_H
#define SPECTROGRAMWINDOW_H

class SpectrogramWindow;
class SpectrogramThread;
class SpectrogramTrigger;

#include "bcbase.h"
#include "spectrogram.h"
#include "mutex.h"

// Waits for triggers from the DSP instance
class SpectrogramTrigger : public Thread
{
public:
	SpectrogramTrigger(Spectrogram *plugin, SpectrogramWindow *window);
	~SpectrogramTrigger();

	void run();

	Spectrogram *plugin;
	Mutex startup_lock;
	SpectrogramWindow *window;
};

class SpectrogramThread : public Thread
{
public:
	SpectrogramThread(Spectrogram *plugin);
	~SpectrogramThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Spectrogram *plugin;
	SpectrogramWindow *window;
};

class SpectrogramCanvas;
class SpectrogramWindowSize;


class SpectrogramWindow : public BC_Window
{
public:
	SpectrogramWindow(Spectrogram *plugin);
	~SpectrogramWindow();
	
	int create_objects();
	int close_event();
	int update_gui();
	int resize_event(int w, int h);
	int update_freq_text(int cursor_x);

	Spectrogram *plugin;
	SpectrogramCanvas *canvas;
	VFrame *bitmap; // The bitmap
	SpectrogramWindowSize *windowsize;
	BC_Title *freq_text;
};

class SpectrogramCanvas : public BC_Canvas
{
public:
	SpectrogramCanvas(Spectrogram *plugin, int x, int y);
	~SpectrogramCanvas();
	
	int cursor_motion();
	
	Spectrogram *plugin;
};

class SpectrogramWindowSize : public BC_TextBox
{
public:
	SpectrogramWindowSize(Spectrogram *plugin, int x, int y);
	~SpectrogramWindowSize();
	int handle_event();
	Spectrogram *plugin;
};


#endif
