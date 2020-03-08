#ifndef LEVELWINDOW_H
#define LEVELWINDOW_H

#include "defaults.inc"
#include "levelwindowgui.inc"
#include "mainwindow.inc"
#include "thread.h"


class LevelWindow : public Thread
{
public:
	LevelWindow(MainWindow *mwindow);
	~LevelWindow();
	
	int create_objects();
	int load_defaults(Defaults *defaults);
	int update_defaults(Defaults *defaults);
	int change_channels(int new_channels);
	int change_format();
	int show_window();
	int hide_window();
	int reset();
	int init_meter(int total_peaks);
	int stop_meter();
	int update(int peak_number, int channel, float value);
	int update_meter(int peak_number, int last_peak, int total_peaks);
	void run();

// allocated according to playback buffers
	float **peak_history;
	int channels;
	
	int horizontal, meter_visible, w, h;
	LevelWindowGUI *gui;
	MainWindow *mwindow;
};





#endif
