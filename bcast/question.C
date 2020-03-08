#include <string.h>
#include "mainwindow.inc"
#include "question.h"




QuestionWindow::QuestionWindow(char *display)
 : BC_Window(display, MEGREY, ICONNAME ": Question", 375, 160, 375, 160)
{
}

QuestionWindow::~QuestionWindow()
{
}

int QuestionWindow::create_objects(char *string, char *string1, int use_cancel)
{
	add_tool(new BC_Title(5, 5, string));
	if(string1) add_tool(new BC_Title(5, 25, string1));
	add_tool(new QuestionYesButton(this));
	add_tool(new QuestionNoButton(this));
	if(use_cancel) add_tool(new QuestionCancelButton(this));
return 0;
}

QuestionYesButton::QuestionYesButton(QuestionWindow *window)
 : BC_BigButton(30, 115, "Yes")
{
	this->window = window;
}

int QuestionYesButton::handle_event()
{
	window->set_done(2);
return 0;
}

int QuestionYesButton::keypress_event()
{
	if(window->get_keypress() == 'y') { handle_event(); return 1; }
	return 0;
return 0;
}

QuestionNoButton::QuestionNoButton(QuestionWindow *window)
 : BC_BigButton(150, 115, "No")
{
	this->window = window;
}

int QuestionNoButton::handle_event()
{
	window->set_done(0);
return 0;
}

int QuestionNoButton::keypress_event()
{
	if(window->get_keypress() == 'n') { handle_event(); return 1; }
	return 0;
return 0;
}

QuestionCancelButton::QuestionCancelButton(QuestionWindow *window)
 : BC_BigButton(250, 115, "Cancel")
{
	this->window = window;
}

int QuestionCancelButton::handle_event()
{
	window->set_done(1);
return 0;
}

int QuestionCancelButton::keypress_event()
{
	if(window->get_keypress() == ESC) { handle_event();  return 1; }
	return 0;
return 0;
}

