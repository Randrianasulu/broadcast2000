#include "sratewindow.h"

SRateWindow::SRateWindow()
 : BC_Window("", MEGREY, "Convert Samplerate", 320, 110, 0, 0)
{ }

SRateWindow::~SRateWindow()
{
	delete ok;
	delete cancel;
	delete rate_text;
}

int SRateWindow::create_objects(int *output_rate)
{
	this->output_rate = output_rate;
	add_tool(new BC_Title(10, 10, "Enter the new sample rate:"));
	add_tool(ok = new SRateWindowOK(this));
	add_tool(cancel = new SRateWindowCancel(this));
	add_tool(rate_text = new SRateWindowRate(this->output_rate));
}

SRateWindowOK::SRateWindowOK(SRateWindow *window) : BC_BigButton(10, 80, "OK")
{ this->window = window; }

int SRateWindowOK::handle_event() { window->set_done(0); }

int SRateWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
}

SRateWindowCancel::SRateWindowCancel(SRateWindow *window) : BC_BigButton(200, 80, "Cancel")
{ this->window = window; }

int SRateWindowCancel::handle_event() { window->set_done(1); }

int SRateWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
}

SRateWindowRate::SRateWindowRate(int *output_rate) : BC_TextBox(5, 25, 200, *output_rate)
{
	this->output_rate = output_rate;
}

SRateWindowRate::~SRateWindowRate()
{
}
	
int SRateWindowRate::handle_event()
{
	*output_rate = atol(get_text());
}
