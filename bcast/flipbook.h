#ifndef FLIPBOOK_H
#define FLIPBOOK_H

#include "bcbase.h"
#include "thread.h"

class FlipBookItem
{
public:
	Asset asset;
	long duration;     // samples or frames depending on the asset
};


class FlipBook : public BC_MenuItem
{
public:
	FlipBook(MainWindow *mwindow);
	~FlipBook();

	handle_event();

	MainWindow *mwindow;
};

class FlipBookThread : public Thread
{
public:
	FlipBookThread(MainWindow *mwindow);
	~FlipBookThread();

	void run();

	MainWindow *mwindow;
};

class FlipBookGUI : public BC_Window
{
public:
	FlipBookGUI(FlipBookThread *thread, MainWindow *mwindow);
	~FlipBookGUI();

	FlipBookThread *thread;
	MainWindow *mwindow;
};




#endif
