#include <string.h>
#include "mainwindow.inc"
#include "recconfirmdelete.h"

RecConfirmDelete::RecConfirmDelete()
 : BC_Window("", MEGREY, ICONNAME ": Confirm", 320, 100, 0, 0)
{
}

RecConfirmDelete::~RecConfirmDelete()
{
	delete ok;
	delete cancel;
}

int RecConfirmDelete::create_objects(char *string)
{
	char string2[256];
	sprintf(string2, "Delete this file and %s?", string);
	add_tool(new BC_Title(5, 5, string2));
	add_tool(ok = new RecConfirmDeleteOK(this));
	add_tool(cancel = new RecConfirmDeleteCancel(this));
return 0;
}


RecConfirmDeleteOK::RecConfirmDeleteOK(RecConfirmDelete *window)
 : BC_BigButton(5, 50, "Yes, delete it")
{ this->window = window; }

RecConfirmDeleteOK::~RecConfirmDeleteOK() {}

int RecConfirmDeleteOK::handle_event() { window->set_done(0); return 0;
};

int RecConfirmDeleteOK::keypress_event() 
{ 
	if(window->get_keypress() == 'y') { handle_event(); return 1; }
	return 0;
return 0;
}



RecConfirmDeleteCancel::RecConfirmDeleteCancel(RecConfirmDelete *window)
 : BC_BigButton(150, 50, "No")
{ this->window = window; }

RecConfirmDeleteCancel::~RecConfirmDeleteCancel() {}

int RecConfirmDeleteCancel::handle_event() { window->set_done(1); return 0;
};

int RecConfirmDeleteCancel::keypress_event() 
{ 
	if(window->get_keypress() == 'n') { handle_event(); return 1; }
	return 0;
return 0;
}










