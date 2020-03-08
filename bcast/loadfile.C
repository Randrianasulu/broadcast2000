#include <string.h>
#include "assets.h"
#include "errorbox.h"
#include "file.h"
#include "filesystem.h"
#include "indexfile.h"
#include "loadfile.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"

Load::Load(MainWindow *mwindow, char *text, char *hotkey_text, char hotkey, int import_)
 : BC_MenuItem(text, hotkey_text, hotkey), Thread()
{ this->mwindow = mwindow; this->import_ = import_; }

int Load::set_mainmenu(MainMenu *mmenu)
{
	this->mmenu = mmenu;
return 0;
}

int Load::set_import(int import)
{
	import_ = import;
return 0;
}

int Load::handle_event() { start(); return 0;
}

void Load::run()
{
	int result;
// ======================================= get path from user
	char directory[1024], filename[1024];
	sprintf(directory, "~");
	mwindow->defaults->get("DIRECTORY", directory);
	LoadFileWindow *window;

	window = new LoadFileWindow(mwindow, mwindow->display, directory);
	window->create_objects();
	result = window->run_window();

	mwindow->defaults->update("DIRECTORY", window->get_directory());

	strcpy(filename, window->get_filename());
	delete window;

// ======================================= try to load it
	if(filename[0] == 0) return;              // no filename given
	if(result == 1) return;          // user cancelled
	mwindow->gui->lock_window();
	mwindow->stop_playback(1);
	mwindow->gui->unlock_window();
	mwindow->interrupt_indexes();
	result = mwindow->load(filename, import_);                    // load it
	return;
}


LoadFileWindow::LoadFileWindow(MainWindow *mwindow, char *display, char *init_directory)
 : BC_FileBox(display, init_directory, "Load", "Select a file to load")
{ this->mwindow = mwindow; }

LoadFileWindow::~LoadFileWindow() {}

LocateFileWindow::LocateFileWindow(MainWindow *mwindow, char *display, char *init_directory, char *old_filename)
 : BC_FileBox(display, init_directory, "Load", old_filename)
{ this->mwindow = mwindow; }

LocateFileWindow::~LocateFileWindow() {}

LoadPrevious::LoadPrevious(Load *loadfile)
 : BC_MenuItem(""), Thread()
{ this->loadfile = loadfile; }

int LoadPrevious::handle_event()
{
// run as thread to prompt users about missing files
	loadfile->mwindow->stop_playback(1);
	loadfile->mwindow->interrupt_indexes();
	start(); 
return 0;
}

void LoadPrevious::run()
{
	loadfile->mwindow->load(path, loadfile->import_);
}

int LoadPrevious::set_path(char *path)
{
	strcpy(this->path, path);
return 0;
}
