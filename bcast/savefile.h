#ifndef SAVEFILE_H
#define SAVEFILE_H

#include "bcbase.h"
#include "mainmenu.inc"
#include "mainwindow.inc"
#include "savefile.inc"

class Save : public BC_MenuItem
{
public:
	Save(MainWindow *mwindow);
	int handle_event();
	int create_objects(SaveAs *saveas);
	int save_before_quit();
	
	int quit_now;
	MainWindow *mwindow;
	SaveAs *saveas;
};

class SaveAs : public BC_MenuItem, public Thread
{
public:
	SaveAs(MainWindow *mwindow);
	int set_mainmenu(MainMenu *mmenu);
	int handle_event();
	void run();
	
	int quit_now;
	MainWindow *mwindow;
	MainMenu *mmenu;
};

class SaveFileWindow : public BC_FileBox
{
public:
	SaveFileWindow(MainWindow *mwindow, char *display, char *init_directory);
	~SaveFileWindow();
	MainWindow *mwindow;
};

#endif
