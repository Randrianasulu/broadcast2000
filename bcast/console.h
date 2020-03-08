#ifndef CONSOLE_H
#define CONSOLE_H

#include "bcbase.h"
#include "console.inc"
#include "consolescroll.inc"
#include "mainwindow.inc"
#include "modules.inc"
#include "thread.h"

class Console : public Thread
{
public:
	Console(MainWindow *mwindow);
	~Console();
	int create_objects(int w, int h, int console_hidden, int vertical);
	int update_defaults(Defaults *defaults);
	int flip_vertical();
	int redo_pixels();

	void run();
// ============================= drawing

	int change_channels(int oldchannels, int newchannels);

// =============================== movement

	int pixelmovement(int distance);

// ============================= editing commands
// track manipulation
 	int add_audio_track();    
 	int add_video_track();    
	int delete_track();
	int delete_project();
	int start_reconfigure(int unlock_window = 0);       // stop the playback for a reconfiguration
	int stop_reconfigure(int unlock_window = 0);        // restart playback after reconfiguration

// playback
	int toggles_selected(int on, int show, int mute);
	int select_all_toggles(int on, int show, int mute);
	int deselect_all_toggles(int on, int show, int mute);

// =============================== accounting

	int button_down, new_status, reconfigure_trigger;
	int pixel_start;
	int vertical;

	ConsoleWindow *gui;
	MainWindow *mwindow;
	Modules *modules;
};

class ConsoleWindow : public BC_Window
{
public:
	ConsoleWindow(MainWindow *mwindow, int w, int h, int console_hidden);
	~ConsoleWindow();

	int create_objects();
	int resize_event(int w, int h);
	int flip_vertical(int w, int h);
	int close_event();
	int keypress_event();
	int button_release();

	MainWindow *mwindow;
	ConsoleMainScroll *scroll;
	Console *console;
};


#endif
