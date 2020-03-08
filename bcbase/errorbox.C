#include <string.h>
#include "errorbox.h"

ErrorBox::ErrorBox(char *display, char *title)
 : BC_Window(display, MEGREY, title, 400, 120, 400, 120)
{
}

ErrorBox::~ErrorBox()
{
	delete ok;
}

int ErrorBox::create_objects(char *text1, char *text2, char *text3, char *text4)
{
	add_tool(new BC_Title(5, 5, text1));
	if(text2) add_tool(new BC_Title(5, 25, text2));
	if(text3) add_tool(new BC_Title(5, 45, text3));
	if(text4) add_tool(new BC_Title(5, 65, text4));
	add_tool(ok = new ErrorBoxOkButton(this));
return 0;
}

ErrorBoxOkButton::ErrorBoxOkButton(ErrorBox *errorbox)
 : BC_BigButton(150, 80, "OK")
{ this->errorbox = errorbox; }

int ErrorBoxOkButton::handle_event()
{
	errorbox->set_done(0);
return 0;
}

int ErrorBoxOkButton::keypress_event()
{
	if(errorbox->get_keypress() == 13 ||
		errorbox->get_keypress() == ESC) handle_event();
return 0;
}
