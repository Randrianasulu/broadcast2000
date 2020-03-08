#include "cdripwindow.h"

CDRipWindow::CDRipWindow(CDRipMain *cdripper)
 : BC_Window("", MEGREY, "CD Ripper", 450, 192, 450, 192)
{ this->cdripper = cdripper; }

CDRipWindow::~CDRipWindow()
{
	delete ok;
	delete cancel;
	delete track1;
	delete min1;
	delete sec1;
	delete track2;
	delete min2;
	delete sec2;
	delete device;
}

int CDRipWindow::create_objects()
{
	int y = 10, x = 10;
	add_tool(new BC_Title(x, y, "Select the range to transfer:")); y += 25;
	add_tool(new BC_Title(x, y, "Track")); x += 70;
	add_tool(new BC_Title(x, y, "Min")); x += 70;
	add_tool(new BC_Title(x, y, "Sec")); x += 100;

	add_tool(new BC_Title(x, y, "Track")); x += 70;
	add_tool(new BC_Title(x, y, "Min")); x += 70;
	add_tool(new BC_Title(x, y, "Sec")); x += 100;
	
	x = 10;  y += 25;
	add_tool(track1 = new CDRipTextValue(this, &(cdripper->track1), x, y, 50));
	x += 70;
	add_tool(min1 = new CDRipTextValue(this, &(cdripper->min1), x, y, 50));
	x += 70;
	add_tool(sec1 = new CDRipTextValue(this, &(cdripper->sec1), x, y, 50));
	x += 100;
	
	add_tool(track2 = new CDRipTextValue(this, &(cdripper->track2), x, y, 50));
	x += 70;
	add_tool(min2 = new CDRipTextValue(this, &(cdripper->min2), x, y, 50));
	x += 70;
	add_tool(sec2 = new CDRipTextValue(this, &(cdripper->sec2), x, y, 50));

	x = 10;   y += 30;
	add_tool(new BC_Title(x, y, "From", LARGEFONT, RED));
	x += 240;
	add_tool(new BC_Title(x, y, "To", LARGEFONT, RED));

	x = 10;   y += 35;
	add_tool(new BC_Title(x, y, "CD Device:"));
	x += 100;
	add_tool(device = new CDRipWindowDevice(this, cdripper->device, x, y, 200));

	x = 10;   y += 35;
	add_tool(ok = new CDRipWindowOK(this, x, y));
	x += 300;
	add_tool(cancel = new CDRipWindowCancel(this, x, y));
}




CDRipWindowOK::CDRipWindowOK(CDRipWindow *window, int x, int y)
 : BC_BigButton(x, y, "OK")
{ this->window = window; }

int CDRipWindowOK::handle_event() { window->set_done(0); }

int CDRipWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
}




CDRipWindowCancel::CDRipWindowCancel(CDRipWindow *window, int x, int y)
 : BC_BigButton(x, y, "Cancel")
{ this->window = window; }

int CDRipWindowCancel::handle_event() { window->set_done(1); }

int CDRipWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
}




CDRipTextValue::CDRipTextValue(CDRipWindow *window, int *output, int x, int y, int w)
 : BC_TextBox(x, y, w, *output)
{
	this->output = output;
	this->window = window;
}

CDRipTextValue::~CDRipTextValue()
{
}
	
int CDRipTextValue::handle_event()
{
	*output = atol(get_text());
}

CDRipWindowDevice::CDRipWindowDevice(CDRipWindow *window, char *device, int x, int y, int w)
 : BC_TextBox(x, y, w, device)
{
	this->window = window;
	this->device = device;
}

CDRipWindowDevice::~CDRipWindowDevice()
{
}

int CDRipWindowDevice::handle_event()
{
	strcpy(device, get_text());
}
