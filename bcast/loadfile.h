#ifndef LOADFILE_H
#define LOADFILE_H

#include "bcbase.h"
#include "mainmenu.inc"
#include "mainwindow.inc"
#include "thread.h"

class Load : public BC_MenuItem, public Thread
{
public:
	Load(MainWindow *mwindow, char *text, char *hotkey_text, char hotkey, int import_);
	int handle_event();
	int set_import(int import);

	void run();
	
	int set_mainmenu(MainMenu *mmenu);
	
	MainWindow *mwindow;
	MainMenu *mmenu;
	int import_;
};

class LoadFileWindow : public BC_FileBox
{
public:
	LoadFileWindow(MainWindow *mwindow, char *display, char *init_directory);
	~LoadFileWindow();
	MainWindow *mwindow;
};

class LocateFileWindow : public BC_FileBox
{
public:
	LocateFileWindow(MainWindow *mwindow, char *display, char *init_directory, char *old_filename);
	~LocateFileWindow();
	MainWindow *mwindow;
};

class LoadPrevious : public BC_MenuItem, public Thread
{
public:
	LoadPrevious(Load *loadfile);
	int handle_event();
	void run();
	
	int set_path(char *path);
	
	Load *loadfile;
	char path[1024];
};

#endif
