#ifndef MAINXSCROLL_H
#define MAINXSCROLL_H

class MainXScrollBar;
class MainYScrollBar;

#include "bcbase.h"
#include "mainwindow.inc"
#include "mainwindowgui.inc"

class MainSampleScroll
{
public:
	MainSampleScroll(MainWindowGUI *gui);
	~MainSampleScroll();

	int create_objects();
	int flip_vertical();
	int in_use();
	int resize_event(int w, int h);
	int set_position();
	int handle_event(long position);
	long oldposition;

private:
	MainWindowGUI *gui;
	MainWindow *mwindow;
	MainXScrollBar *xscroll;
	MainYScrollBar *yscroll;
};

class MainXScrollBar : public BC_XScrollBar
{
public:
	MainXScrollBar(MainWindowGUI *gui, MainSampleScroll *scroll);
	~MainXScrollBar();
	int handle_event();

private:
	MainWindowGUI *gui;
	MainSampleScroll *scroll;
};


class MainYScrollBar : public BC_YScrollBar
{
public:
	MainYScrollBar(MainWindowGUI *gui, MainSampleScroll *scroll);
	~MainYScrollBar();
	int handle_event();

private:
	MainWindowGUI *gui;
	MainSampleScroll *scroll;
};






#endif
