#ifndef QUIT_H
#define QUIT_H

#include "bcbase.h"
#include "mainwindow.inc"
#include "savefile.inc"

class Quit : public BC_MenuItem, public Thread
{
public:
	Quit(MainWindow *mwindow);
	int create_objects(Save *save);
	int handle_event();
	void run();
	
	Save *save;
	MainWindow *mwindow;
};

#endif
