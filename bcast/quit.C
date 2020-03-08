#include <string.h>
#include "assets.h"
#include "buttonbar.h"
#include "confirmquit.h"
#include "console.h"
#include "errorbox.h"
#include "levelwindow.h"
#include "levelwindowgui.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "quit.h"
#include "record.h"
#include "render.h"
#include "savefile.h"
#include "videowindow.h"
#include "videowindowgui.h"

Quit::Quit(MainWindow *mwindow)
 : BC_MenuItem("Quit", "q", 'q'), Thread() 
{ this->mwindow = mwindow; }
int Quit::create_objects(Save *save)
{ this->save = save; return 0;
}

int Quit::handle_event() 
{
	mwindow->stop_playback();
	if(mwindow->changes_made ||
		mwindow->gui->buttonbar->record_button->in_progress ||
		mwindow->gui->mainmenu->render->in_progress) start(); 
	else 
	{        // quit
		if(mwindow->gui)
		{
			mwindow->interrupt_indexes();
			mwindow->gui->set_done(0);
			mwindow->console->gui->set_done(0);
		}
	}
return 0;
}

void Quit::run()
{
	int result = 0;
// Test execution conditions
	if(mwindow->gui->buttonbar->record_button->in_progress)
	{
		ErrorBox error;
		error.create_objects("Can't quit while a recording is in progress.");
		error.run_window();
		return;
	}
	else
	if(mwindow->gui->mainmenu->render->in_progress)
	{
		ErrorBox error;
		error.create_objects("Can't quit while a render is in progress.");
		error.run_window();
		return;
	}
	

// Quit
	{
		ConfirmQuitWindow confirm(mwindow->display);
		confirm.create_objects("Save edit list before exiting?");
		result = confirm.run_window();
	}
	switch(result)
	{
		case 0:          // quit
			if(mwindow->gui)
			{
				mwindow->interrupt_indexes();
// core dumps when mwindow is killed first
				mwindow->console->gui->set_done(0);
				mwindow->level_window->gui->set_done(0);
				mwindow->video_window->gui->set_done(0);
// Last command in program
				mwindow->gui->set_done(0);
			}
			break;
			
		case 1:        // cancel
			return;
			break;
		
		case 2:           // save
			save->save_before_quit(); 
			return;
			break;
	}
}
