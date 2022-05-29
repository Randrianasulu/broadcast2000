#include "fratewindow.h"

FRateWindow::FRateWindow()
 : BC_Window("", MEGREY, "Convert Framerate", 320, 110, 0, 0)
{ }

FRateWindow::~FRateWindow()
{
	delete ok;
	delete cancel;
	delete rate_text;
}

int FRateWindow::create_objects(float *output_rate)
{
	this->output_rate = output_rate;
	add_tool(new BC_Title(10, 10, "Enter the new frame rate:"));
	add_tool(ok = new FRateWindowOK(this));
	add_tool(cancel = new FRateWindowCancel(this));
	add_tool(rate_text = new FRateWindowRate(this->output_rate));
return 0;
}

FRateWindowOK::FRateWindowOK(FRateWindow *window) : BC_BigButton(10, 80, "OK")
{ this->window = window; }

int FRateWindowOK::handle_event() { window->set_done(0); return 0; }

int FRateWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
}

FRateWindowCancel::FRateWindowCancel(FRateWindow *window) : BC_BigButton(200, 80, "Cancel")
{ this->window = window; }

int FRateWindowCancel::handle_event() { window->set_done(1); return 0; }

int FRateWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
}

FRateWindowRate::FRateWindowRate(float *output_rate) : BC_TextBox(5, 25, 200, *output_rate)
{
	this->output_rate = output_rate;
}

FRateWindowRate::~FRateWindowRate()
{
}
	
int FRateWindowRate::handle_event()
{
	*output_rate = atof(get_text());
return 0;
}
