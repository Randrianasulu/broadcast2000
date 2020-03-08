#include <string.h>
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "tracks.h"
#include "trackscroll.h"

TrackScroll::TrackScroll(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	this->gui = mwindow->gui;
	xscroll = 0;
	yscroll = 0;
}

TrackScroll::~TrackScroll()
{
}

int TrackScroll::create_objects(int top, int bottom)
{
	if(gui)
	{
		if(mwindow->tracks_vertical)
		{ gui->add_tool(xscroll = new TrackXScroll(mwindow, bottom, gui->get_h() - 41, top - bottom, 17)); }
		else
		{ gui->add_tool(yscroll = new TrackYScroll(mwindow, gui->get_w() - 17, top, 17, bottom - top));}
	}
return 0;
}

int TrackScroll::update()
{
	if(gui)
	{
		if(xscroll)
			xscroll->set_position(mwindow->tracks->totalpixels() + mwindow->zoom_track, mwindow->tracks->view_start, mwindow->tracks->vertical_pixels());
		else
		if(yscroll)
			yscroll->set_position(mwindow->tracks->totalpixels() + mwindow->zoom_track, mwindow->tracks->view_start, mwindow->tracks->vertical_pixels());
	}
return 0;
}

int TrackScroll::resize_event(int w, int h, int top, int bottom)
{
	if(gui)
	{
		if(xscroll)
		xscroll->set_size(bottom, h - 41, top - bottom, 17); // fix scrollbar
		else
		if(yscroll)
		yscroll->set_size(w - 17, top, 17, bottom - top); // fix scrollbar
		
		update();
	}
return 0;
}

int TrackScroll::flip_vertical(int top, int bottom)
{
	if(gui)
	{
		if(mwindow->tracks_vertical)
		{
			delete yscroll;
			yscroll = 0;
			gui->add_tool(xscroll = new TrackXScroll(mwindow, bottom, gui->get_h() - 41, top - bottom, 17));
		}
		else
		{
			delete xscroll;
			xscroll = 0;
			gui->add_tool(yscroll = new TrackYScroll(mwindow, gui->get_w() - 17, top, 17, bottom - top));
		}
		update();
	}
return 0;
}



TrackXScroll::TrackXScroll(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_XScrollBar(x, y, w, h, 0, 0, 0)
{
	this->mwindow = mwindow;
}
TrackXScroll::~TrackXScroll()
{
}
int TrackXScroll::handle_event()
{
	mwindow->tracks->trackmovement(get_distance());
return 0;
}




TrackYScroll::TrackYScroll(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_YScrollBar(x, y, w, h, 0, 0, 0)
{
	this->mwindow = mwindow;
}
TrackYScroll::~TrackYScroll()
{
}
int TrackYScroll::handle_event()
{
	mwindow->tracks->trackmovement(get_distance());
return 0;
}

