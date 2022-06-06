#include <string.h>
#include "confirmsave.h"
#include "mainwindow.inc"

ConfirmSave::ConfirmSave()
{
}

ConfirmSave::~ConfirmSave()
{
}

int ConfirmSave::test_file(const char *display, const char *path)
{
	FILE *in;
	if(in = fopen(path, "rb"))
	{
		fclose(in);
		ConfirmSaveWindow cwindow(display, path);
		cwindow.create_objects();
		int result = cwindow.run_window();
		return result;
	}
	return 0;
return 0;
}






ConfirmSaveWindow::ConfirmSaveWindow(const char *display, char *filename)
 : BC_Window(display, MEGREY, ICONNAME ": File Exists", 375, 160, 375, 160)
{
	this->filename = filename;
}

ConfirmSaveWindow::~ConfirmSaveWindow()
{
	delete ok;
	delete cancel;
}

int ConfirmSaveWindow::create_objects()
{
	char string[1024];
	sprintf(string, "Overwrite %s?", filename);
	add_tool(new BC_Title(5, 5, string));
	add_tool(ok = new ConfirmSaveOkButton(this));
	add_tool(cancel = new ConfirmSaveCancelButton(this));
return 0;
}

ConfirmSaveOkButton::ConfirmSaveOkButton(ConfirmSaveWindow *nwindow)
 : BC_BigButton(30, 115, "OK")
{
	this->nwindow = nwindow;
}

int ConfirmSaveOkButton::handle_event()
{
	nwindow->set_done(0);
return 0;
}

int ConfirmSaveOkButton::keypress_event()
{
	if(nwindow->get_keypress() == 13) { handle_event();  return 1; }
	return 0;
return 0;
}

ConfirmSaveCancelButton::ConfirmSaveCancelButton(ConfirmSaveWindow *nwindow)
 : BC_BigButton(250, 115, "Cancel")
{
	this->nwindow = nwindow;
}

int ConfirmSaveCancelButton::handle_event()
{
	nwindow->set_done(1);
return 0;
}

int ConfirmSaveCancelButton::keypress_event()
{
	if(nwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

