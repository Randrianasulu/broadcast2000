#include <string.h>
#include "filehtal.h"
#include "mainmenu.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"

MainUndo::MainUndo(MainWindow *mwindow)
{ this->mwindow = mwindow; }

MainUndo::~MainUndo()
{
}

int MainUndo::update_undo_all(const char *description, int after)
{
	update_undo(description, "ALL", after);
return 0;
}

int MainUndo::update_undo_audio(const char *description, int after)
{
	update_undo(description, "AUDIO", after);
return 0;
}

int MainUndo::update_undo_edits(const char *description, int after)
{
	update_undo(description, "EDITS", after);
return 0;
}

int MainUndo::update_undo_patches(const char *description, int after)
{
	update_undo(description, "PATCHES", after);
return 0;
}

int MainUndo::update_undo_console(const char *description, int after)
{
	update_undo(description, "CONSOLE", after);
return 0;
}

int MainUndo::update_undo_timebar(const char *description, int after)
{
	update_undo(description, "TIMEBAR", after);
return 0;
}

int MainUndo::update_undo_automation(const char *description, int after)
{
	update_undo(description, "AUTOMATION", after);
return 0;
}

int MainUndo::update_undo(const char *description, const char *type, int after)
{
	FileHTAL htal;
	mwindow->save(&htal, 0);

	if(!after)
	{
// this is the before buffer
		current_entry = undo_stack.push();
		current_entry->set_type(type);
		current_entry->set_data_before(htal.string);
	}
	else
	{
		current_entry->set_data_after(htal.string);
	}

// the after update is always without a description
	if(!after)
		current_entry->set_description(description);

	if(mwindow->gui)
	{
// the after update is always without a description
		if(!after)
			mwindow->gui->mainmenu->undo->update_caption(description);

		mwindow->gui->mainmenu->redo->update_caption("");
	}
return 0;
}






int MainUndo::undo()
{
	if(undo_stack.current)
	{
		current_entry = undo_stack.current;
		if(current_entry->description && mwindow->gui) 
			mwindow->gui->mainmenu->redo->update_caption(current_entry->description);
		
		FileHTAL htal;
		
		htal.read_from_string(current_entry->data_before);
		load_from_undo(&htal, current_entry->type);
		
		undo_stack.pull();    // move current back
		if(mwindow->gui)
		{
			current_entry = undo_stack.current;
			if(current_entry)
				mwindow->gui->mainmenu->undo->update_caption(current_entry->description);
			else
				mwindow->gui->mainmenu->undo->update_caption("");
		}
	}
return 0;
}

int MainUndo::redo()
{
	current_entry = undo_stack.pull_next();
	
	if(current_entry)
	{
		FileHTAL htal;
		htal.read_from_string(current_entry->data_after);
		load_from_undo(&htal, current_entry->type);

		if(mwindow->gui)
		{
			mwindow->gui->mainmenu->undo->update_caption(current_entry->description);
			
			if(current_entry->next)
				mwindow->gui->mainmenu->redo->update_caption(current_entry->next->description);
			else
				mwindow->gui->mainmenu->redo->update_caption("");
		}
	}
return 0;
}


int MainUndo::load_from_undo(FileHTAL *htal, const char *type)
{
	mwindow->interrupt_indexes();
	if(!strcmp(type, "ALL"))
	{
		mwindow->delete_project(0);          // load entire project
		mwindow->load(htal);
	}
	else
	if(!strcmp(type, "AUDIO"))
	{
		mwindow->delete_project(0);          // load audio which is really everything
		mwindow->load(htal);
	}
	else
	if(!strcmp(type, "EDITS"))
	{
		mwindow->load_edits(htal);
	}
	else
	if(!strcmp(type, "PATCHES"))
	{
		mwindow->load_patches(htal);
	}
	else
	if(!strcmp(type, "CONSOLE"))
	{
		mwindow->load_console(htal);
	}
	else
	if(!strcmp(type, "TIMEBAR"))
	{
		mwindow->load_timebar(htal);
	}
	else
	if(!strcmp(type, "AUTOMATION"))
	{
		mwindow->load_automation(htal);
	}
return 0;
}




