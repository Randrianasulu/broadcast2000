#ifndef TIMEBAR_H
#define TIMEBAR_H

#include "bcbase.h"
#include "filehtal.inc"
#include "labels.inc"
#include "mainwindow.inc"
#include "recordlabel.inc"

class TimeBarCanvas;
class TimeBarLeftArrow;
class TimeBarRightArrow;
class TimeBarUpArrow;
class TimeBarDownArrow;

class TimeBar
{
public:
	TimeBar(MainWindow *mwindow);
	~TimeBar();

	int create_objects();
	int update_defaults();
	int flip_vertical();

// ================================= file operations

	int save(FileHTAL *htal);
	int load(FileHTAL *htal);

	int delete_project();        // clear timebar of labels

	int draw();                  // draw everything over
	int samplemovement();
	int resize_event(int w, int h);
	int refresh_labels();
	int clear_labels(long start, long end);
	int toggle_label();
	int toggle_label(long position);

// ========================================= editing

	int copy(long start, long end, FileHTAL *htal);
	int paste(long start, long end, long sample_length, FileHTAL *htal);
	int paste_output(long startproject, long endproject, long startsource, long endsource, RecordLabels *new_labels);
	int clear(long start, long end);
	int paste_silence(long start, long end);
	int modify_handles(long oldposition, long newposition, int currentend);
	int stop_playback();
	int select_region(long sample);

	MainWindow *mwindow;
	Labels *labels;	
	TimeBarGUI *gui;

private:
	int draw_bevel();
};

class TimeBarGUI : public BC_SubWindow
{
public:
	TimeBarGUI(MainWindow *mwindow, int x, int y, int w, int h);
	~TimeBarGUI();

	int create_objects();
	int resize_event(int w, int h);
	int flip_vertical(int w, int h);
	int delete_arrows();    // for flipping vertical

	TimeBarCanvas *canvas;
	TimeBarLeftArrow *left_arrow;
	TimeBarRightArrow *right_arrow;
	TimeBarUpArrow *up_arrow;
	TimeBarDownArrow *down_arrow;
	MainWindow *mwindow;
};

class TimeBarCanvas : public BC_Canvas
{
public:
	TimeBarCanvas(MainWindow *mwindow, int x, int y, int w, int h);
	~TimeBarCanvas();

	int button_press();

	MainWindow *mwindow;
};

class TimeBarLeftArrow : public BC_LeftTriangleButton
{
public:
	TimeBarLeftArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h);
	~TimeBarLeftArrow();

	int handle_event();

	MainWindow *mwindow;
	TimeBarGUI *gui;
};

class TimeBarRightArrow : public BC_RightTriangleButton
{
public:
	TimeBarRightArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h);
	~TimeBarRightArrow();

	int handle_event();

	MainWindow *mwindow;
	TimeBarGUI *gui;
};

class TimeBarUpArrow : public BC_UpTriangleButton
{
public:
	TimeBarUpArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h);
	~TimeBarUpArrow();

	int handle_event();

	MainWindow *mwindow;
	TimeBarGUI *gui;
};

class TimeBarDownArrow : public BC_DownTriangleButton
{
public:
	TimeBarDownArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h);
	~TimeBarDownArrow();

	int handle_event();

	MainWindow *mwindow;
	TimeBarGUI *gui;
};

#endif
