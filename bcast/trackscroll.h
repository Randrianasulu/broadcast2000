#ifndef TRACKSCROLL_H
#define TRACKSCROLL_H

class TrackXScroll;
class TrackYScroll;


#include "bcbase.h"
#include "mainwindow.inc"
#include "mainwindowgui.inc"

class TrackScroll
{
public:
	TrackScroll(MainWindow *mwindow);
	~TrackScroll();

	int create_objects(int top, int bottom);
	int resize_event(int w, int h, int top, int bottom);
	int flip_vertical(int top, int bottom);
	int update();               // reflect new track view

	MainWindowGUI *gui;
	MainWindow *mwindow;
	TrackYScroll *yscroll;
	TrackXScroll *xscroll;
};

class TrackXScroll : public BC_XScrollBar
{
public:
	TrackXScroll(MainWindow *mwindow, int x, int y, int w, int h);
	~TrackXScroll();
	int handle_event();
	MainWindow *mwindow;
};

class TrackYScroll : public BC_YScrollBar
{
public:
	TrackYScroll(MainWindow *mwindow, int x, int y, int w, int h);
	~TrackYScroll();
	int handle_event();
	MainWindow *mwindow;
};


#endif
