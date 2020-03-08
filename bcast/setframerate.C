#include <string.h>
#include "mainundo.h"
#include "mainwindow.h"
#include "playbackengine.h"
#include "preferences.h"
#include "setframerate.h"
#include "tracks.h"

SetFrameRate::SetFrameRate(MainWindow *mwindow)
 : BC_MenuItem("Frame rate...")
{ 
	this->mwindow = mwindow;
	thread = new SetFrameRateThread(mwindow);
}

SetFrameRate::~SetFrameRate()
{
	delete thread;
}
 
int SetFrameRate::handle_event() { thread->start(); return 0;
}

SetFrameRateThread::SetFrameRateThread(MainWindow *mwindow) : Thread()
{
	this->mwindow = mwindow;
}

SetFrameRateThread::~SetFrameRateThread() {}

void SetFrameRateThread::run()
{
	frame_rate = mwindow->frame_rate;
//	scale_data = mwindow->defaults->get("FRAMERATESCALEDATA", 0);
	scale_data = 1;

	SetFrameRateWindow window(this);
	window.create_objects();
	int result = window.run_window();
	if(!result)
	{
		mwindow->undo->update_undo_edits("Frame rate", 0);
		if(scale_data) mwindow->tracks->scale_time(frame_rate / mwindow->frame_rate, 1, 1, 1, 0, mwindow->tracks->total_samples());
		mwindow->frame_rate = frame_rate;
		mwindow->preferences->actual_frame_rate = frame_rate;
		mwindow->undo->update_undo_edits();
		mwindow->redraw_time_dependancies();
	}

//	mwindow->defaults->update("FRAMERATESCALEDATA", scale_data);
}


SetFrameRateWindow::SetFrameRateWindow(SetFrameRateThread *thread)
 : BC_Window("", MEGREY, ICONNAME ": Frame Rate", 340, 140, 340, 140)
{
	this->thread = thread;
}

SetFrameRateWindow::~SetFrameRateWindow()
{
}

int SetFrameRateWindow::create_objects()
{
	add_tool(new BC_Title(5, 5, "Enter the frame rate to use:"));
	add_tool(new SetFrameRateOkButton(this));
	add_tool(new SetFrameRateCancelButton(this));

	add_tool(new SetFrameRateTextBox(thread));
//	add_tool(new SetFrameRateMoveData(thread));
return 0;
}

SetFrameRateOkButton::SetFrameRateOkButton(SetFrameRateWindow *frwindow)
 : BC_BigButton(5, 80, "OK")
{
	this->frwindow = frwindow;
}

int SetFrameRateOkButton::handle_event()
{
	frwindow->set_done(0);
return 0;
}

int SetFrameRateOkButton::keypress_event()
{
	if(frwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

SetFrameRateCancelButton::SetFrameRateCancelButton(SetFrameRateWindow *frwindow)
 : BC_BigButton(90, 80, "Cancel")
{
	this->frwindow = frwindow;
}

int SetFrameRateCancelButton::handle_event()
{
	frwindow->set_done(1);
return 0;
}

int SetFrameRateCancelButton::keypress_event()
{
	if(frwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

SetFrameRateTextBox::SetFrameRateTextBox(SetFrameRateThread *thread)
 : BC_TextBox(10, 40, 100, thread->frame_rate)
{
	this->thread = thread;
}

int SetFrameRateTextBox::handle_event()
{
	thread->frame_rate = atof(get_text());
return 0;
}

SetFrameRateMoveData::SetFrameRateMoveData(SetFrameRateThread *thread)
 : BC_CheckBox(120, 40, 17, 17, thread->scale_data, "Scale data")
{
	this->thread = thread;
}

int SetFrameRateMoveData::handle_event()
{
	thread->scale_data = get_value();
return 0;
}
