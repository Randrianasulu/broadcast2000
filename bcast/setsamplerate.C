#include <string.h>
#include "mainwindow.h"
#include "setsamplerate.h"

SetSampleRate::SetSampleRate(MainWindow *mwindow)
 : BC_MenuItem("Sample rate...", ""), Thread() 
{ this->mwindow = mwindow; }
 
int SetSampleRate::handle_event() { start(); return 0;
}

void SetSampleRate::run()
{
	SetSampleRateWindow window;
	window.create_objects(mwindow);
	window.run_window();
}


SetSampleRateWindow::SetSampleRateWindow(char *display)
 : BC_Window(display, MEGREY, ICONNAME ": Sample Rate", 340, 140, 340, 140)
{
}

SetSampleRateWindow::~SetSampleRateWindow()
{
	delete ok;
	delete cancel;
	delete text;
}

int SetSampleRateWindow::create_objects(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	
	add_tool(new BC_Title(5, 5, "Enter the sample rate to use:"));
	add_tool(ok = new SetSampleRateOkButton(this));
	add_tool(cancel = new SetSampleRateCancelButton(this));
	
	char string[1024];
	sprintf(string, "%d", mwindow->sample_rate);
	sample_rate = mwindow->sample_rate;
	add_tool(text = new SetSampleRateTextBox(this, string));
return 0;
}

SetSampleRateOkButton::SetSampleRateOkButton(SetSampleRateWindow *srwindow)
 : BC_BigButton(5, 80, "OK")
{
	this->srwindow = srwindow;
	mwindow = srwindow->mwindow;
}

int SetSampleRateOkButton::handle_event()
{
// exclude invalid entry
	if(srwindow->sample_rate >= 4000 && srwindow->sample_rate <= 200000)
	{
		mwindow->sample_rate = srwindow->sample_rate;
		mwindow->changes_made = 1;
		mwindow->redraw_time_dependancies();
	}
	
	srwindow->set_done(0);
return 0;
}

int SetSampleRateOkButton::keypress_event()
{
	if(srwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

SetSampleRateCancelButton::SetSampleRateCancelButton(SetSampleRateWindow *srwindow)
 : BC_BigButton(90, 80, "Cancel")
{
	this->srwindow = srwindow;
}

int SetSampleRateCancelButton::handle_event()
{
	srwindow->set_done(1);
return 0;
}

int SetSampleRateCancelButton::keypress_event()
{
	if(srwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

SetSampleRateTextBox::SetSampleRateTextBox(SetSampleRateWindow *srwindow, char *text)
 : BC_TextBox(10, 40, 100, text)
{
	this->srwindow = srwindow;
}

int SetSampleRateTextBox::handle_event()
{
	srwindow->sample_rate = atol(get_text());
return 0;
}
