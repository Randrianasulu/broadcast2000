#include <string.h>
#include "confirmsave.h"
#include "errorbox.h"
#include "file.h"
#include "filehtal.h"
#include "fileformat.h"
#include "indexfile.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "savefile.h"

Save::Save(MainWindow *mwindow) : BC_MenuItem("Save", "s", 's')
{ this->mwindow = mwindow; quit_now = 0; }

int Save::create_objects(SaveAs *saveas)
{
	this->saveas = saveas;
return 0;
}

int Save::handle_event()
{
	if(mwindow->filename[0] == 0) saveas->start();
	else
	{
// save it
		FileHTAL htal;
		mwindow->save(&htal, 1);

		if(htal.write_to_file(mwindow->filename))
		{
			char string2[256];
			sprintf(string2, "Couldn't open %s.", mwindow->filename);
			ErrorBox error(mwindow->display);
			error.create_objects(string2);
			error.run_window();
			return 1;		
		}
		mwindow->changes_made = 0;
		if(saveas->quit_now) mwindow->gui->set_done(0);
	}
return 0;
}

int Save::save_before_quit()
{
	saveas->quit_now = 1;
	handle_event();
return 0;
}

SaveAs::SaveAs(MainWindow *mwindow)
 : BC_MenuItem("Save as...", ""), Thread()
{ this->mwindow = mwindow; quit_now = 0; }

int SaveAs::set_mainmenu(MainMenu *mmenu)
{
	this->mmenu = mmenu;
return 0;
}

int SaveAs::handle_event() { quit_now = 0; start(); return 0;
}

void SaveAs::run()
{
// ======================================= get path from user
	int result;
	char directory[1024], filename[1024];
	sprintf(directory, "~");
	mwindow->defaults->get("DIRECTORY", directory);

// Loop if file exists
	do{
		SaveFileWindow *window;

		window = new SaveFileWindow(mwindow, mwindow->display, directory);
		window->create_objects();
		result = window->run_window();
		mwindow->defaults->update("DIRECTORY", window->get_directory());
		strcpy(filename, window->get_filename());
		delete window;

// Extend the filename with .htal
		if(strlen(filename) < 5 || strcasecmp(&filename[strlen(filename) - 5], ".htal"))
		{
			strcat(filename, ".htal");
		}

// ======================================= try to save it
		if(filename[0] == 0) return;              // no filename given
		if(result == 1) return;          // user cancelled
		FILE *in;
		if(in = fopen(filename, "rb"))
		{
			fclose(in);
			ConfirmSaveWindow cwindow(mwindow->display, filename);
			cwindow.create_objects();
			int result2 = cwindow.run_window();
			if(result2) result = 1;
		}
	}while(result);        // file exists so repeat

// save it
	FileHTAL htal;
	mwindow->set_filename(filename);      // update the project name
	mwindow->save(&htal, 1);

	if(htal.write_to_file(filename))
	{
		char string2[256];
		mwindow->set_filename("");      // update the project name
		sprintf(string2, "Couldn't open %s.", filename);
		ErrorBox error(mwindow->display);
		error.create_objects(string2);
		error.run_window();
		return;		
	}
	mwindow->changes_made = 0;
	mmenu->add_load(filename);
	if(quit_now) mwindow->gui->set_done(0);
	return;
}

SaveFileWindow::SaveFileWindow(MainWindow *mwindow, char *display, char *init_directory)
 : BC_FileBox(display, init_directory, "Save", "Enter a filename to save as")
{ this->mwindow = mwindow; }

SaveFileWindow::~SaveFileWindow() {}

