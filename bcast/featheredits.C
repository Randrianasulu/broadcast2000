#include <string.h>
#include "featheredits.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"




FeatherEdits::FeatherEdits(MainWindow *mwindow, int audio, int video)
 : BC_MenuItem("Feather Edits..."), Thread()
{ this->mwindow = mwindow; this->audio = audio; this->video = video; }

int FeatherEdits::handle_event()
{
	Thread::synchronous = 0;
	Thread::start();
return 0;
}


void FeatherEdits::run()
{
	int result;
	long feather_samples;

	{
		feather_samples = mwindow->get_feather(audio, video);

		FeatherEditsWindow window(feather_samples);

		window.create_objects(audio, video);
		result = window.run_window();
		feather_samples = window.feather_samples;
	}

	if(!result)
	{
		mwindow->gui->lock_window();
		mwindow->undo->update_undo_edits("Feather", 0);
		
		mwindow->feather_edits(feather_samples, audio, video);

		mwindow->undo->update_undo_edits();
		mwindow->gui->unlock_window();
	}
}


FeatherEditsWindow::FeatherEditsWindow(long feather_samples)
 : BC_Window("", MEGREY, ICONNAME ": Feather Edits", 340, 140, 340, 140)
{
	this->feather_samples = feather_samples;
}

FeatherEditsWindow::~FeatherEditsWindow()
{
	delete ok;
	delete cancel;
	delete text;
}

int FeatherEditsWindow::create_objects(int audio, int video)
{
	this->audio = audio;
	this->video = video;

	if(audio)
		add_tool(new BC_Title(5, 5, "Feather by how many samples:"));
	else
		add_tool(new BC_Title(5, 5, "Feather by how many frames:"));

	add_tool(ok = new FeatherEditsOkButton(this));
	add_tool(cancel = new FeatherEditsCancelButton(this));
		
	char string[1024];
	sprintf(string, "%d", feather_samples);
	add_tool(text = new FeatherEditsTextBox(this, string));
return 0;
}

FeatherEditsOkButton::FeatherEditsOkButton(FeatherEditsWindow *window)
 : BC_BigButton(5, 80, "OK")
{
	this->window = window;
}

int FeatherEditsOkButton::handle_event()
{
	window->set_done(0);
return 0;
}

int FeatherEditsOkButton::keypress_event()
{
	if(window->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

FeatherEditsCancelButton::FeatherEditsCancelButton(FeatherEditsWindow *window)
 : BC_BigButton(90, 80, "Cancel")
{
	this->window = window;
}

int FeatherEditsCancelButton::handle_event()
{
	window->set_done(1);
return 0;
}

int FeatherEditsCancelButton::keypress_event()
{
	if(window->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

FeatherEditsTextBox::FeatherEditsTextBox(FeatherEditsWindow *window, char *text)
 : BC_TextBox(10, 40, 100, text)
{
	this->window = window;
}

int FeatherEditsTextBox::handle_event()
{
	window->feather_samples = atol(get_text());
return 0;
}
