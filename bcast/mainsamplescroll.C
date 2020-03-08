#include <string.h>
#include "mainsamplescroll.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "tracks.h"

MainSampleScroll::MainSampleScroll(MainWindowGUI *gui)
{
	this->gui = gui;
	mwindow = gui->mwindow;
	oldposition = 0;
	xscroll = 0;
	yscroll = 0;
}

MainSampleScroll::~MainSampleScroll()
{
	//printf("~MainSampleScroll\n");
}

int MainSampleScroll::create_objects()
{
	if(mwindow->tracks_vertical)
	gui->add_tool(yscroll = new MainYScrollBar(gui, this));
	else
	gui->add_tool(xscroll = new MainXScrollBar(gui, this));
return 0;
}

int MainSampleScroll::flip_vertical()
{
	if(mwindow->tracks_vertical)
	{
		delete xscroll;
		xscroll = 0;
		gui->add_tool(yscroll = new MainYScrollBar(gui, this));
	}
	else
	{
		delete yscroll;
		yscroll = 0;
		gui->add_tool(xscroll = new MainXScrollBar(gui, this));
	}
return 0;
}

int MainSampleScroll::in_use()
{
	if(mwindow->tracks_vertical)
	return yscroll->in_use();
	else
	return xscroll->in_use();
return 0;
}


int MainSampleScroll::resize_event(int w, int h)
{
	if(xscroll) xscroll->set_size(0, h - 41, w, 17);
	else
	if(yscroll) yscroll->set_size(w - 17, 25, 17, h - 25 - 24);
return 0;
}

int MainSampleScroll::set_position()
{
	if(xscroll) xscroll->set_position(mwindow->tracks->total_samples() + mwindow->tracks->view_samples(), mwindow->view_start, mwindow->tracks->view_samples());
	else
	if(yscroll) yscroll->set_position(mwindow->tracks->total_samples() + mwindow->tracks->view_samples(), mwindow->view_start, mwindow->tracks->view_samples());

	oldposition = mwindow->view_start;
return 0;
}

int MainSampleScroll::handle_event(long position)
{
	if(position != oldposition)
	{
		position /= mwindow->zoom_sample;
		position *= mwindow->zoom_sample;
		
		if(xscroll) xscroll->set_position(mwindow->tracks->total_samples() + mwindow->tracks->view_samples(), position, mwindow->tracks->view_samples());
		else
		if(yscroll) yscroll->set_position(mwindow->tracks->total_samples() + mwindow->tracks->view_samples(), position, mwindow->tracks->view_samples());

		long distance = position - oldposition;
		oldposition = position;
		
		mwindow->samplemovement(distance);
	}
return 0;
}






MainXScrollBar::MainXScrollBar(MainWindowGUI *gui, MainSampleScroll *scroll)
 : BC_XScrollBar(0, gui->get_h() - 41, gui->get_w(), 17, 0, 0, 0)
{
	this->gui = gui;
	this->scroll = scroll;
}

MainXScrollBar::~MainXScrollBar()
{
}

int MainXScrollBar::handle_event()
{
	scroll->handle_event(get_position());
return 0;
}


MainYScrollBar::MainYScrollBar(MainWindowGUI *gui, MainSampleScroll *scroll)
 : BC_YScrollBar(gui->get_w() - 17, 25, 17, gui->get_h() - 25 - 24, 0, 0, 0)
{
	this->gui = gui;
	this->scroll = scroll;
}

MainYScrollBar::~MainYScrollBar()
{
	//scroll->handle_event(get_position());
}

int MainYScrollBar::handle_event()
{
	scroll->handle_event(get_position());
return 0;
}

