#include <string.h>
#include "bckeys.h"
#include "bcprogress.h"
#include "bcprogressbox.h"
#include "bctitle.h"
#include "bcwindow.h"

BC_ProgressBox::BC_ProgressBox(const char *display, const char *text, long length, int cancel_button)
 : Thread(1)                    // create synchronous
{
	pwindow = new BC_ProgressWindow(display, cancel_button);
	pwindow->create_objects(text, length);
	this->cancel_button = cancel_button;
}

BC_ProgressBox::~BC_ProgressBox()
{
	delete pwindow;
}

void BC_ProgressBox::run()
{
	pwindow->run_window();
}

int BC_ProgressBox::update(long position)
{
	return pwindow->update(position);
return 0;
}

int BC_ProgressBox::update_title(char *title)
{
	pwindow->caption->update(title);
return 0;
}

int BC_ProgressBox::update_length(long length)
{
	pwindow->bar->update_length(length);
return 0;
}


int BC_ProgressBox::cancelled()
{
	return pwindow->cancelled;
return 0;
}

int BC_ProgressBox::stop_progress()
{
	pwindow->set_done(0);
	//Thread::end();
	Thread::join();
return 0;
}

BC_ProgressWindow::BC_ProgressWindow(const char *display, int cancel_button)
 : BC_Window(display, MEGREY, "Progress", 340, 120, 340, 120)
{
	cancelled = 0;
	cancel = 0;
	this->cancel_button = cancel_button;
}

BC_ProgressWindow::~BC_ProgressWindow()
{
	if(cancel_button) delete cancel;
}

int BC_ProgressWindow::create_objects(const char *text, long length)
{
	this->text = text;
	add_tool(caption = new BC_Title(5, 5, text));
	add_tool(bar = new BC_ProgressBar(5, 35, 330, 30, length));
	if(cancel_button) add_tool(cancel = new BC_ProgressWindowCancelButton(this));
	else
	{
		add_tool(caption = new BC_Title(5, 75, "This process may not be interrupted.", MEDIUMFONT, RED));
	}
return 0;
}

int BC_ProgressWindow::update(long position)
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

BC_ProgressWindowCancelButton::BC_ProgressWindowCancelButton(BC_ProgressWindow *pwindow)
 : BC_BigButton(130, 80, "Cancel")
{ this->pwindow = pwindow; }

int BC_ProgressWindowCancelButton::handle_event()
{
	pwindow->set_done(1);
	pwindow->cancelled = 1;
return 0;
}

int BC_ProgressWindowCancelButton::keypress_event()
{
	if(pwindow->get_keypress() == ESC) handle_event();
return 0;
}
