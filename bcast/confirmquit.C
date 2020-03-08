#include <string.h>
#include "confirmquit.h"
#include "mainwindow.inc"

ConfirmQuitWindow::ConfirmQuitWindow(char *display)
 : BC_Window(display, MEGREY, ICONNAME ": Question", 375, 160, 375, 160)
{
}

ConfirmQuitWindow::~ConfirmQuitWindow()
{
	delete yes;
	delete no;
	delete cancel;
}

int ConfirmQuitWindow::create_objects(char *string)
{
	add_tool(new BC_Title(5, 5, string));
	add_tool(new BC_Title(25, 30, "( Answering ""No"" will destroy changes )"));
	add_tool(yes = new ConfirmQuitYesButton(this));
	add_tool(no = new ConfirmQuitNoButton(this));
	add_tool(cancel = new ConfirmQuitCancelButton(this));
return 0;
}

ConfirmQuitYesButton::ConfirmQuitYesButton(ConfirmQuitWindow *window)
 : BC_BigButton(30, 115, "Yes")
{
	this->window = window;
}

int ConfirmQuitYesButton::handle_event()
{
	window->set_done(2);
return 0;
}

int ConfirmQuitYesButton::keypress_event()
{
	if(window->get_keypress() == 'y') { handle_event(); return 1; }
	return 0;
return 0;
}

ConfirmQuitNoButton::ConfirmQuitNoButton(ConfirmQuitWindow *window)
 : BC_BigButton(150, 115, "No")
{
	this->window = window;
}

int ConfirmQuitNoButton::handle_event()
{
	window->set_done(0);
return 0;
}

int ConfirmQuitNoButton::keypress_event()
{
	if(window->get_keypress() == 'n') { handle_event(); return 1; }
	return 0;
return 0;
}

ConfirmQuitCancelButton::ConfirmQuitCancelButton(ConfirmQuitWindow *window)
 : BC_BigButton(250, 115, "Cancel")
{
	this->window = window;
}

int ConfirmQuitCancelButton::handle_event()
{
	window->set_done(1);
return 0;
}

int ConfirmQuitCancelButton::keypress_event()
{
	if(window->get_keypress() == ESC) handle_event();
return 0;
}

