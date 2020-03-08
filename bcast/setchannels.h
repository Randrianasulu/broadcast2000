#ifndef SETCHANNELS_H
#define SETCHANNELS_H


#include "bcbase.h"
#include "mainwindow.inc"
#include "maxchannels.h"
#include "thread.h"


class SetChannels : public BC_MenuItem, public Thread
{
public:
	SetChannels(MainWindow *mwindow);
	int handle_event();
	void run();
	
	int old_channels, new_channels;
	int channel_positions[MAXCHANNELS];
	MainWindow *mwindow;
};

class SetChannelsOkButton;
class SetChannelsCancelButton;
class SetChannelsTextBox;
class SetChannelsCanvas;

class SetChannelsWindow : public BC_Window
{
public:
	SetChannelsWindow(SetChannels *setchannels);
	~SetChannelsWindow();
	int create_objects();
	
	SetChannels *setchannels;
	SetChannelsOkButton *ok;
	SetChannelsCancelButton *cancel;
	SetChannelsTextBox *text;
	SetChannelsCanvas *canvas;
};

class SetChannelsOkButton : public BC_BigButton
{
public:
	SetChannelsOkButton(SetChannelsWindow *window);
	
	int handle_event();
	int keypress_event();
	
	SetChannelsWindow *window;
};

class SetChannelsCancelButton : public BC_BigButton
{
public:
	SetChannelsCancelButton(SetChannelsWindow *window);
	
	int handle_event();
	int keypress_event();
	
	SetChannelsWindow *window;
};

class SetChannelsTextBox : public BC_TextBox
{
public:
	SetChannelsTextBox(SetChannels *setchannels, SetChannelsCanvas *canvas, char *text);
	
	int handle_event();
	
	SetChannels *setchannels;
	MainWindow *mwindow;
	SetChannelsCanvas *canvas;
};


class SetChannelsCanvas : public BC_Canvas
{
public:
	SetChannelsCanvas(SetChannels *setchannels, int x, int y, int w, int h);
	~SetChannelsCanvas();
	
	int create_objects();
	int draw(int angle = -1);
	int get_dimensions(int channel_position, int &x, int &y, int &w, int &h);
	int button_press();
	int button_release();
	int cursor_motion();

private:	
	int active_channel;   // for selection
	int degree_offset;
	int box_r;
	
	int poltoxy(int &x, int &y, int r, int d);
	int xytopol(int &d, int x, int y);
	SetChannels *setchannels;
};


#endif

