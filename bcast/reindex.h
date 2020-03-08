#ifndef REINDEX_H
#define REINDEX_H

class ReIndex;

#include "indexfile.h"
#include "bcbase.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "thread.h"

class ReIndex : public BC_MenuItem, public Thread
{
public:
	ReIndex(MainWindow *mwindow);
	~ReIndex();
	void run();
	handle_event();
	
	MainWindow *mwindow;
};

class ReIndexOkButton;
class ReIndexCancelButton;
class ReIndexTextBox;

class ReIndexWindow : public BC_Window
{
public:
	ReIndexWindow(char *display = "");
	~ReIndexWindow();
	create_objects();
	
	MainWindow *mwindow;
	long sample_rate;
	ReIndexOkButton *ok;
	ReIndexCancelButton *cancel;
};

class ReIndexOkButton : public BC_BigButton
{
public:
	ReIndexOkButton(ReIndexWindow *window);
	
	handle_event();
	keypress_event();
	
	ReIndexWindow *window;
};

class ReIndexCancelButton : public BC_BigButton
{
public:
	ReIndexCancelButton(ReIndexWindow *window);
	
	handle_event();
	keypress_event();
	
	ReIndexWindow *window;
};

#endif
