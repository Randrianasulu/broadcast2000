#ifndef SYNTHESIZERWINDOW_H
#define SYNTHESIZERWINDOW_H

#include "bcbase.h"
#include "filehtal.h"
#include "headers.h"
#include "mutex.h"

class SynthThread : public Thread
{
public:
	SynthThread(Synth *synth);
	~SynthThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Synth *synth;
	SynthWindow *window;
};


class SynthWindow : public BC_Window
{
public:
	SynthWindow(Synth *synth);
	~SynthWindow();
	
	int create_objects();
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);
	int close_event();
	int resize_event(int w, int h);
	int update_gui();
	SynthOscGUI* add_oscillator(SynthOscillator *oscillator, int y);
	int relocate_oscillators(int position);
	int waveform_to_text(char *text, int waveform);
	int update_scrollbar();

	float canvas_zoom;
	int view_y;
	Synth *synth;
	SynthFreqPot *freqpot;
	SynthCanvas *canvas;
	SynthWaveForm *waveform;
	SynthBaseFreq *base_freq;
	SynthMenu *menu;
	SynthClear *clear;
	SynthSubWindow *subwindow;
	SynthScroll *scroll;
	SynthClear *clearbutton;
};

class SynthCanvas : public BC_Canvas
{
public:
	SynthCanvas(Synth *synth, SynthWindow *window, int x, int y, int w, int h);
	~SynthCanvas();
	
	int update();
	Synth *synth;
	SynthWindow *window;
};

class SynthCanvasZoomin : public BC_DownTriangleButton
{
public:
	SynthCanvasZoomin(SynthWindow *window, int x, int y);
	~SynthCanvasZoomin();
	int handle_event();
	SynthWindow *window;
};

class SynthCanvasZoomout : public BC_UpTriangleButton
{
public:
	SynthCanvasZoomout(SynthWindow *window, int x, int y);
	~SynthCanvasZoomout();
	int handle_event();
	SynthWindow *window;
};

class SynthSubWindow : public BC_SubWindow
{
public:
	SynthSubWindow(Synth *synth, int x, int y, int w, int h);
	~SynthSubWindow();

	Synth *synth;
};

class SynthWaveForm : public BC_PopupMenu
{
public:
	SynthWaveForm(Synth *synth, int x, int y, char *text);
	~SynthWaveForm();

	int add_items();
	Synth *synth;
};

class SynthWaveFormItem : public BC_PopupItem
{
public:
	SynthWaveFormItem(Synth *synth, char *text, int value);
	~SynthWaveFormItem();
	
	int handle_event();
	
	int value;
	Synth *synth;
};

class SynthBaseFreq : public BC_TextBox
{
public:
	SynthBaseFreq(Synth *synth, int x, int y);
	~SynthBaseFreq();
	int handle_event();
	Synth *synth;
	SynthFreqPot *freq_pot;
};

class SynthFreqPot : public BC_QPot
{
public:
	SynthFreqPot(Synth *synth, SynthWindow *window, int x, int y);
	~SynthFreqPot();
	int handle_event();
	SynthWindow *window;
	Synth *synth;
	SynthBaseFreq *freq_text;
};

class SynthClear : public BC_BigButton
{
public:
	SynthClear(Synth *synth, int x, int y);
	~SynthClear();
	int handle_event();
	Synth *synth;
};

class SynthScroll : public BC_YScrollBar
{
public:
	SynthScroll(Synth *synth, SynthWindow *window, int x, int y, int h);
	~SynthScroll();
	
	int handle_event();
	
	Synth *synth;
	SynthWindow *window;
};

class SynthAddOsc : public BC_BigButton
{
public:
	SynthAddOsc(Synth *synth, SynthWindow *window, int x, int y);
	~SynthAddOsc();
	
	int handle_event();
	
	Synth *synth;
	SynthWindow *window;
};


class SynthDelOsc : public BC_BigButton
{
public:
	SynthDelOsc(Synth *synth, SynthWindow *window, int x, int y);
	~SynthDelOsc();
	
	int handle_event();
	
	Synth *synth;
	SynthWindow *window;
};









class SynthOscGUI
{
public:
	SynthOscGUI(SynthWindow *window, SynthOscillator *oscillator);
	~SynthOscGUI();

	int create_objects(int view_y);

	SynthOscGUILevel *level;
	SynthOscGUIPhase *phase;
	SynthOscGUIFreq *freq;
	BC_Title *title;

	SynthWindow *window;
	SynthOscillator *oscillator;
};

class SynthOscGUILevel : public BC_FPot
{
public:
	SynthOscGUILevel(SynthOscillator *oscillator, int y);
	~SynthOscGUILevel();

	int handle_event();

	SynthOscillator *oscillator;
};

class SynthOscGUIPhase : public BC_IPot
{
public:
	SynthOscGUIPhase(SynthOscillator *oscillator, int y);
	~SynthOscGUIPhase();

	int handle_event();

	SynthOscillator *oscillator;
};

class SynthOscGUIFreq : public BC_IPot
{
public:
	SynthOscGUIFreq(SynthOscillator *oscillator, int y);
	~SynthOscGUIFreq();

	int handle_event();

	SynthOscillator *oscillator;
};




#endif
