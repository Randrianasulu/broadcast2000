#ifndef NEW_H
#define NEW_H

class NewWindow;
class NewOkButton;
class NewCancelButton;
class NewTracks;
class NewChannels;
class NewRate;
class NewOutputW;
class NewOutputH;
class NewAspectW;
class NewAspectH;

#include "bcbase.h"
#include "defaults.inc"
#include "mainwindow.inc"
#include "thread.h"


class New : public BC_MenuItem, public Thread
{
public:
	New(MainWindow *mwindow);
	int handle_event();
	int set_script(FileHTAL *script);
	int run_script(FileHTAL *script);

	void run();
	int load_defaults();
	int save_defaults();
	int update_aspect(NewWindow *nwindow);

	MainWindow *mwindow;
	FileHTAL *script;
	int tracks, sample_rate, channels;
	int vtracks, track_w, track_h, output_w, output_h;
	float aspect_w, aspect_h;
	float frame_rate;
	int auto_aspect;
	int already_running;
};

class NewWindow : public BC_Window
{
public:
	NewWindow(New *new_thread, char *display);
	~NewWindow();
	
	int create_objects();

	NewOutputW *output_w_button;
	NewOutputH *output_h_button;
	NewAspectW *aspect_w_text;
	NewAspectH *aspect_h_text;
	New *new_thread;
};

class NewOkButton : public BC_BigButton
{
public:
	NewOkButton(NewWindow *nwindow);
	~NewOkButton();

	int handle_event();
	int keypress_event();
	
	NewWindow *nwindow;
};

class NewCancelButton : public BC_BigButton
{
public:
	NewCancelButton(NewWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	NewWindow *nwindow;
};

class NewCloneButton : public BC_BigButton
{
public:
	NewCloneButton(NewWindow *nwindow);
	int handle_event();
	NewWindow *nwindow;
};

class NewTracks : public BC_TextBox
{
public:
	NewTracks(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewVTracks : public BC_TextBox
{
public:
	NewVTracks(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewChannels : public BC_TextBox
{
public:
	NewChannels(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewRate : public BC_TextBox
{
public:
	NewRate(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewFrameRate : public BC_TextBox
{
public:
	NewFrameRate(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewTrackW : public BC_TextBox
{
public:
	NewTrackW(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewTrackH : public BC_TextBox
{
public:
	NewTrackH(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewOutputW : public BC_TextBox
{
public:
	NewOutputW(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewOutputH : public BC_TextBox
{
public:
	NewOutputH(NewWindow *nwindow, char *text);
	
	int handle_event();
	
	NewWindow *nwindow;
};

class NewAspectAuto : public BC_CheckBox
{
public:
	NewAspectAuto(NewWindow *nwindow);
	~NewAspectAuto();
	int handle_event();
	NewWindow *nwindow;
};

class NewAspectW : public BC_TextBox
{
public:
	NewAspectW(NewWindow *nwindow, char *text);
	int handle_event();
	NewWindow *nwindow;
};

class NewAspectH : public BC_TextBox
{
public:
	NewAspectH(NewWindow *nwindow, char *text);
	int handle_event();
	NewWindow *nwindow;
};

#endif
