#include <string.h>
#include "mainwindow.inc"
#include "progressbox.h"

ProgressBox::ProgressBox(const char *display, const char *text, long length, int cancel_button)
 : Thread(1)                    // create synchronous
{
	pwindow = new ProgressWindow(display, cancel_button);
	pwindow->create_objects(text, length);
	this->cancel_button = cancel_button;
}

ProgressBox::~ProgressBox()
{
	delete pwindow;
}

void ProgressBox::run()
{
	pwindow->run_window();
}

int ProgressBox::stop_progress()
{
	pwindow->set_done(0);
	//Thread::end();
	Thread::join();
return 0;
}


int ProgressBox::update(long position)
{
	return pwindow->update(position);
return 0;
}

int ProgressBox::update_title(char *new_title)
{
	pwindow->title->update(new_title);
return 0;
}

int ProgressBox::update_length(long new_length)
{
	pwindow->bar->update_length(new_length);
return 0;
}

int ProgressBox::cancelled()
{
	return pwindow->cancelled;
return 0;
}

ProgressWindow::ProgressWindow(const char *display, int cancel_button)
 : BC_Window(display, MEGREY, ICONNAME ": Progress", 340, 120, 340, 120)
{
	cancelled = 0;
	cancel = 0;
	this->cancel_button = cancel_button;
}

ProgressWindow::~ProgressWindow()
{
//	if(cancel_button) delete cancel;
}

int ProgressWindow::create_objects(const char *text, long length)
{
	this->text = text;
	add_tool(title = new BC_Title(5, 5, text));
	add_tool(bar = new BC_ProgressBar(5, 35, 330, 30, length));
	if(cancel_button) add_tool(cancel = new ProgressWindowCancelButton(this));
	else
	{
		add_tool(new BC_Title(5, 75, "This process may not be interrupted.", MEDIUMFONT, RED));
	}
return 0;
}

int ProgressWindow::update(long position)
{
	if(!cancelled)
	{
		lock_window();
		bar->update(position);
		unlock_window();
	}
	return cancelled;
return 0;
}

ProgressWindowCancelButton::ProgressWindowCancelButton(ProgressWindow *pwindow)
 : BC_BigButton(130, 80, "Cancel")
{ this->pwindow = pwindow; }

int ProgressWindowCancelButton::handle_event()
{
	pwindow->set_done(1);
	pwindow->cancelled = 1;
return 0;
}

int ProgressWindowCancelButton::keypress_event()
{
	if(pwindow->get_keypress() == ESC) handle_event();
return 0;
}
