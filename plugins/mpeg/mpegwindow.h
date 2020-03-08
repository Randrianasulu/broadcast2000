#ifndef MPEGWINDOW_H
#define MPEGWINDOW_H

#include "mpeg.inc"
#include "bcbase.h"

class MpegVideoWindow;
class MpegAudioWindow;

class MpegAudioThread : public Thread
{
public:
	MpegAudioThread(MpegMain *plugin);
	~MpegAudioThread();
	
	void run();
	
	MpegMain *plugin;
	MpegAudioWindow *window;
	int running;
};

class MpegAudioWindow : public BC_Window
{
public:
	MpegAudioWindow(MpegAudioThread *thread);
	~MpegAudioWindow();
	
	int initialize();
	
	MpegAudioThread *thread;
};

class OKButton : public BC_BigButton
{
public:
	OKButton(int x, int y, BC_Window *window);
	int handle_event();
	int keypress_event();
	BC_Window *window;
};

class BitrateText : public BC_TextBox
{
public:
	BitrateText(int x, int y, int *output);
	~BitrateText();
	int handle_event();
	int *output;
};

class Interlaced : public BC_CheckBox
{
public:
	Interlaced(int x, int y, MpegMain *plugin);
	~Interlaced();

	int handle_event();
	MpegMain *plugin;
};

class MPEG1 : public BC_Radial
{
public:
	MPEG1(int x, int y, MpegVideoWindow *window, MpegMain *plugin);
	~MPEG1();

	int handle_event();
	MpegVideoWindow *window;
	MpegMain *plugin;
};

class MPEG2 : public BC_Radial
{
public:
	MPEG2(int x, int y, MpegVideoWindow *window, MpegMain *plugin);
	~MPEG2();

	int handle_event();
	MpegVideoWindow *window;
	MpegMain *plugin;
};

class MPEG4 : public BC_Radial
{
public:
	MPEG4(int x, int y, MpegVideoWindow *window, MpegMain *plugin);
	~MPEG4();

	int handle_event();
	MpegVideoWindow *window;
	MpegMain *plugin;
};

class MpegVideoThread : public Thread
{
public:
	MpegVideoThread(MpegMain *plugin);
	~MpegVideoThread();
	
	void run();
	
	MpegMain *plugin;
	MpegVideoWindow *window;
	int running;
};

class MpegVideoWindow : public BC_Window
{
public:
	MpegVideoWindow(MpegVideoThread *thread);
	~MpegVideoWindow();
	
	int initialize();
	int set_mpeg(int value);
	
	MpegVideoThread *thread;
	MPEG1 *mpeg1;
	MPEG2 *mpeg2;
	MPEG4 *mpeg4;
};

#endif
