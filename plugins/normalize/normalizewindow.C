#include "normalizewindow.h"

NormalizeWindow::NormalizeWindow()
 : BC_Window("", MEGREY, "Normalize", 320, 110, 0, 0)
{ }

NormalizeWindow::~NormalizeWindow()
{
	delete ok;
	delete cancel;
	delete overload_text;
	delete separate_tracks_toggle;
}

int NormalizeWindow::create_objects(float *db_over, int *separate_tracks)
{
	this->db_over = db_over;
	this->separate_tracks = separate_tracks;
	add_tool(new BC_Title(10, 10, "Enter the DB to overload by:"));
	add_tool(ok = new NormalizeWindowOK(this));
	add_tool(cancel = new NormalizeWindowCancel(this));
	add_tool(overload_text = new NormalizeWindowOverload(this->db_over));
	add_tool(new BC_Title(40, 60, "Treat tracks independantly"));
	add_tool(separate_tracks_toggle = new NormalizeWindowSeparate(this->separate_tracks));
return 0;
}

NormalizeWindowOK::NormalizeWindowOK(NormalizeWindow *window) : BC_BigButton(10, 80, "OK")
{ this->window = window; }

int NormalizeWindowOK::handle_event() { window->set_done(0); return 0; }

int NormalizeWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
}

NormalizeWindowCancel::NormalizeWindowCancel(NormalizeWindow *window) : BC_BigButton(200, 80, "Cancel")
{ this->window = window; }

int NormalizeWindowCancel::handle_event() { window->set_done(1); return 0; }

int NormalizeWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
}

NormalizeWindowOverload::NormalizeWindowOverload(float *db_over) : BC_TextBox(5, 25, 200, *db_over)
{
	this->db_over = db_over;
}

NormalizeWindowOverload::~NormalizeWindowOverload()
{
}
	
int NormalizeWindowOverload::handle_event()
{
	*db_over = atof(get_text());
return 0;
}


NormalizeWindowSeparate::NormalizeWindowSeparate(int *separate_tracks)
 : BC_CheckBox(10, 60, 15, 15, *separate_tracks)
{
	this->separate_tracks = separate_tracks;
}

NormalizeWindowSeparate::~NormalizeWindowSeparate()
{
}
	
int NormalizeWindowSeparate::handle_event()
{
	*separate_tracks = get_value();
return 0;
}
