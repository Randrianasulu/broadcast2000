#include <string.h>
#include "okbutton.h"

OKButton::OKButton(int x, int y) : BC_BigButton(x, y, "OK")
{
}

OKButton::~OKButton()
{
}

int OKButton::handle_event()
{
	set_done(0);
return 0;
}

int OKButton::keypress_event()
{
	if(get_keypress() == 13)
	{
		set_done(0);
		trap_keypress();
	}
	else
	return 0;
return 0;
}

CancelButton::CancelButton(int x, int y) : BC_BigButton(x, y, "Cancel")
{
}

CancelButton::~CancelButton()
{
}

int CancelButton::handle_event()
{
	set_done(1);
return 0;
}

int CancelButton::keypress_event()
{
	if(get_keypress() == ESC)
	{
		set_done(1);
		trap_keypress();
	}
	else
	return 0;
return 0;
}
