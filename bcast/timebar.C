#include <string.h>
#include "filehtal.h"
#include "labels.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "preferences.h"
#include "recordlabel.h"
#include "timebar.h"
#include "tracks.h"

TimeBar::TimeBar(MainWindow *mwindow)
{
	gui = 0;
	this->mwindow = mwindow;
}

TimeBar::~TimeBar()
{
	delete labels;
}

int TimeBar::create_objects()
{
	Defaults *defaults = mwindow->defaults;
	int x, y, w, h;

	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			x = mwindow->gui->get_w() - TIMEBAR_PIXELS - 17;
			y = mwindow->gui->menu_h();
			w = TIMEBAR_PIXELS;
			h = mwindow->gui->get_h() - mwindow->gui->menu_h() - STATUSBAR_PIXELS;
		}
		else
		{
			x = 0;
			y = mwindow->gui->menu_h() + 30;
			w = mwindow->gui->get_w();
			h = TIMEBAR_PIXELS;
		}
		mwindow->gui->add_subwindow(gui = new TimeBarGUI(mwindow, x, y, w, h));
	}

	labels = new Labels(mwindow, this);
return 0;
}

int TimeBar::delete_project()
{
	labels->delete_all();
return 0;
}

int TimeBar::save(FileHTAL *htal)
{
	labels->save(htal);
return 0;
}

int TimeBar::load(FileHTAL *htal)
{
	labels->load(htal);
return 0;
}



int TimeBar::clear_labels(long start, long end)
{
	labels->clear(start, end, 0);
return 0;
}

int TimeBar::toggle_label()
{
	long position1 = mwindow->selectionstart;
	long position2 = mwindow->selectionend;

	if(mwindow->cursor_on_frames)
		position1 = mwindow->align_to_frames(position1);
	else
		position1 = position1;

	if(mwindow->cursor_on_frames)
		position2 = mwindow->align_to_frames(position2);
	else
		position2 = position2;


	labels->toggle_label(position1, position2);
return 0;
}

int TimeBar::toggle_label(long position)
{
	if(mwindow->cursor_on_frames)
		position = mwindow->align_to_frames(position);

	labels->toggle_label(position, position);
return 0;
}


int TimeBar::resize_event(int w, int h)
{
	int new_x, new_y, new_w, new_h;
	if(gui)
	{
		if(mwindow->tracks_vertical)
		{
			new_x = w - 17 - gui->get_w();
			new_y = gui->get_y();
			new_w = gui->get_w();
			new_h = h - mwindow->gui->menu_h() - STATUSBAR_PIXELS;
			gui->resize_window(new_x, new_y, new_w, new_h);
		}
		else
		{
			new_x = 0;
			new_y = gui->get_y();
			new_w = w;
			new_h = gui->get_h();
			gui->resize_window(new_x, new_y, new_w, new_h);
		}
	}

	gui->resize_event(new_w, new_h);
	labels->resize_event(new_x);
// redrawn by mainwindow
return 0;
}

int TimeBar::flip_vertical()
{
	if(gui)
	{
		gui->flip_vertical(mwindow->gui->get_w(), mwindow->gui->get_h());
		labels->flip_vertical();
		draw();
	}
return 0;
}

int TimeBar::draw()
{
	if(gui)
	{
// pop up all the unselected labels
		labels->draw();
		
// draw the box
		gui->canvas->clear_box(0, 0, gui->canvas->get_w(), gui->canvas->get_h());
		
// fit the time
		static long windowspan, timescale, timescale_, sample_rate, sample;
		static float timescale__;
		static int pixel;
		static char string[256];
		const int TIMESPACING = 100;
		
		sample_rate = mwindow->sample_rate;
		windowspan = mwindow->tracks->view_samples();
		timescale_ = TIMESPACING * mwindow->zoom_sample;
	
		if(timescale_ <= sample_rate / 4) timescale = sample_rate / 4;
		else
		if(timescale_ <= sample_rate / 2) timescale = sample_rate / 2;
		else
		if(timescale_ <= sample_rate) timescale = sample_rate;
		else
		if(timescale_ <= sample_rate * 10) timescale = sample_rate * 10;
		else
		if(timescale_ <= sample_rate * 60) timescale = sample_rate * 60;
		else
		if(timescale_ <= sample_rate * 600) timescale = sample_rate * 600;
		else
		timescale = sample_rate * 3600;
	
		for(timescale__ = timescale; timescale__ > timescale_; timescale__ /= 2)
		;
	
		timescale = (long)(timescale__ * 2);
	
		sample = mwindow->view_start;
		
		sample /= timescale;
		sample *= timescale;
		pixel = (sample - mwindow->view_start) / mwindow->zoom_sample;
		pixel += (mwindow->tracks_vertical ? PATCHBAYHEIGHT : PATCHBAYWIDTH) - TIMEBAR_PIXELS;

// draw the time
		if(mwindow->tracks_vertical)
		{
			for(; pixel < gui->canvas->get_h(); pixel += timescale / mwindow->zoom_sample, sample += timescale)
			{
				totext(string, 
					sample, 
					sample_rate, 
					mwindow->preferences->time_format, 
					mwindow->frame_rate,
					mwindow->preferences->frames_per_foot);
				gui->canvas->draw_vertical_text(0, pixel + 4, string, BLACK, MEGREY);
	// vertical text in X?
	 			gui->canvas->set_color(DKGREY);
				gui->canvas->draw_line(3, pixel, gui->canvas->get_w() - 4, pixel);
	 			gui->canvas->set_color(WHITE);
				gui->canvas->draw_line(3, pixel + 1, gui->canvas->get_w() - 4, pixel + 1);
			}
		}
		else
		{
			for(; pixel < gui->canvas->w; pixel += timescale / mwindow->zoom_sample, sample += timescale)
			{
				totext(string, 
					sample, 
					sample_rate, 
					mwindow->preferences->time_format, 
					mwindow->frame_rate,
					mwindow->preferences->frames_per_foot);
	 			gui->canvas->set_color(BLACK);
				gui->canvas->draw_text(pixel + 4, gui->canvas->get_h() - 4, string);
	 			gui->canvas->set_color(DKGREY);
				gui->canvas->draw_line(pixel, 3, pixel, gui->canvas->get_h() - 4);
	 			gui->canvas->set_color(WHITE);
				gui->canvas->draw_line(pixel + 1, 3, pixel + 1, gui->canvas->get_h() - 4);
			}
		}
		draw_bevel();
		gui->canvas->flash();
	}
return 0;
}

int TimeBar::draw_bevel()
{
	int x1, y1, w, h;
	int lx,ly,ux,uy;

	x1 = y1 = 0;
	w = gui->canvas->get_w(); h = gui->canvas->get_h();
	h--; w--;

	lx = x1+1;  ly = y1+1;
	ux = x1+w-1;  uy = y1+h-1;

	gui->canvas->set_color(gui->canvas->get_resources()->get_bg_light1());
	gui->canvas->draw_line(x1, y1, x1+w, y1);
	gui->canvas->draw_line(x1, y1, x1, y1+h);
	gui->canvas->set_color(gui->canvas->get_resources()->get_bg_light2());
	gui->canvas->draw_line(lx, ly, ux, ly);
	gui->canvas->draw_line(lx, ly, lx, uy);

	gui->canvas->set_color(gui->canvas->get_resources()->get_bg_shadow2());
	gui->canvas->draw_line(x1+w, y1, x1+w, y1+h);
	gui->canvas->draw_line(x1, y1+h, x1+w, y1+h);
	gui->canvas->set_color(gui->canvas->get_resources()->get_bg_shadow1());
	gui->canvas->draw_line(ux, ly, ux, uy);
	gui->canvas->draw_line(lx, uy, ux, uy);
return 0;
}

int TimeBar::samplemovement()
{
	if(gui)
	{
		labels->samplemovement();
		draw();
		gui->canvas->flash();
	}
return 0;
}

int TimeBar::select_region(long sample)
{
	Label *start, *end;

	for(end = labels->first; end && end->position < sample; end = end->next)
		;

	for(start = labels->last; start && start->position >= sample; start = start->previous)
		;

	if(start && end)
	{
		mwindow->set_selection(start->position, end->position);
	}
	else
	if(start)
	{
		if(!mwindow->tracks->total_samples()) return 0;
		mwindow->set_selection(start->position, mwindow->tracks->total_samples());
	}
	else
	if(end)
	{
		mwindow->set_selection(0, end->position);
	}
	mwindow->stop_playback(1);
return 0;
}

int TimeBar::copy(long start, long end, FileHTAL *htal)
{
	labels->copy(start, end, htal);
return 0;
}

int TimeBar::paste(long start, long end, long sample_length, FileHTAL *htal)
{
	labels->paste(start, end, sample_length, htal);
return 0;
}

int TimeBar::paste_output(long startproject, long endproject, long startsource, long endsource, RecordLabels *new_labels)
{
	labels->paste_output(startproject, endproject, startsource, endsource, new_labels);
return 0;
}

int TimeBar::clear(long start, long end)
{ labels->clear(start, end); return 0;
}

int TimeBar::paste_silence(long start, long end)
{ labels->paste_silence(start, end); return 0;
}

int TimeBar::modify_handles(long oldposition, long newposition, int currentend)
{ labels->modify_handles(oldposition, newposition, currentend); return 0;
}

int TimeBar::stop_playback()
{ mwindow->stop_playback(1); return 0;
}









TimeBarGUI::TimeBarGUI(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY)
{
	left_arrow = 0;
	right_arrow = 0;
	up_arrow = 0;
	down_arrow = 0;
	canvas = 0;
	this->mwindow = mwindow;
}

TimeBarGUI::~TimeBarGUI()
{
}

int TimeBarGUI::create_objects()
{
	int w, h;
	w = get_w();
	h = get_h();
	if(mwindow->tracks_vertical)
	{
		add_tool(canvas = new TimeBarCanvas(mwindow, 0, w, w, h - w * 2));
		add_tool(up_arrow = new TimeBarUpArrow(mwindow, this, 0, 0, w, w));
		add_tool(down_arrow = new TimeBarDownArrow(mwindow, this, 0, 0 + h - w, w, w));
	}
	else
	{
		add_tool(canvas = new TimeBarCanvas(mwindow, h, 0, w - h * 2, h));
		add_tool(left_arrow = new TimeBarLeftArrow(mwindow, this, 0, 0, h, h));
		add_tool(right_arrow = new TimeBarRightArrow(mwindow, this, 0 + w - h, 0, h, h));
	}
return 0;
}

int TimeBarGUI::resize_event(int w, int h)
{
	if(mwindow->tracks_vertical)
	{
		canvas->set_size(0, w, w, h - w * 2);
		up_arrow->resize_tool(0, 0);
		down_arrow->resize_tool(0, h - w);
	}
	else
	{
		canvas->set_size(h, 0, w - h * 2, h);
		left_arrow->resize_tool(0, 0);
		right_arrow->resize_tool(w - h, 0);
	}
return 0;
}

int TimeBarGUI::flip_vertical(int w, int h)
{
	delete_arrows();

	if(mwindow->tracks_vertical)
	{
		resize_window(w - 17 - TIMEBAR_PIXELS, mwindow->gui->menu_h(), TIMEBAR_PIXELS, h - mwindow->gui->menu_h() - STATUSBAR_PIXELS);
		canvas->set_size(0, TIMEBAR_PIXELS, TIMEBAR_PIXELS, h - mwindow->gui->menu_h() - STATUSBAR_PIXELS - TIMEBAR_PIXELS * 2);
		add_tool(up_arrow = new TimeBarUpArrow(mwindow, this, 0, 0, get_w(), get_w()));
		add_tool(down_arrow = new TimeBarDownArrow(mwindow, this, 0, get_h() - TIMEBAR_PIXELS, get_w(), get_w()));
	}
	else
	{
		resize_window(0, mwindow->gui->menu_h() + 30, w, TIMEBAR_PIXELS);
		canvas->set_size(TIMEBAR_PIXELS, 0, w - TIMEBAR_PIXELS * 2, TIMEBAR_PIXELS);
		add_tool(left_arrow = new TimeBarLeftArrow(mwindow, this, 0, 0, get_h(), get_h()));
		add_tool(right_arrow = new TimeBarRightArrow(mwindow, this, get_w() - get_h(), 0, get_h(), get_h()));
	}
return 0;
}


int TimeBarGUI::delete_arrows()
{
	if(left_arrow) delete left_arrow;
	if(right_arrow) delete right_arrow;
	if(up_arrow) delete up_arrow;
	if(down_arrow) delete down_arrow;
	left_arrow = 0;
	right_arrow = 0;
	up_arrow = 0;
	down_arrow = 0;
return 0;
}



TimeBarCanvas::TimeBarCanvas(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, MEGREY)
{
	this->mwindow = mwindow;
}

TimeBarCanvas::~TimeBarCanvas()
{
}

int TimeBarCanvas::button_press()
{
	if(get_double_click())
	{
		long sample = (long)(mwindow->tracks_vertical ? 
			(get_cursor_y() - PATCHBAYHEIGHT + get_w()) : 
			(get_cursor_x() - PATCHBAYWIDTH + get_h())) * mwindow->zoom_sample + mwindow->view_start;
		mwindow->timebar->select_region(sample);
		return 1;
	}
	return 0;
return 0;
}

TimeBarLeftArrow::TimeBarLeftArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h)
 : BC_LeftTriangleButton(x, y, w, h, 1)
{ this->mwindow = mwindow; this->gui = gui; }

TimeBarLeftArrow::~TimeBarLeftArrow() {}

int TimeBarLeftArrow::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->prev_label();
return 0;
}



TimeBarRightArrow::TimeBarRightArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h)
 : BC_RightTriangleButton(x, y, w, h, 1)
{ this->mwindow = mwindow; this->gui = gui; }

TimeBarRightArrow::~TimeBarRightArrow() {}

int TimeBarRightArrow::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->next_label();
return 0;
}


TimeBarUpArrow::TimeBarUpArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h)
 : BC_UpTriangleButton(x, y, w, h, 1)
{ this->mwindow = mwindow; this->gui = gui; }

TimeBarUpArrow::~TimeBarUpArrow() {}

int TimeBarUpArrow::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->prev_label();
return 0;
}



TimeBarDownArrow::TimeBarDownArrow(MainWindow *mwindow, TimeBarGUI *gui, int x, int y, int w, int h)
 : BC_DownTriangleButton(x, y, w, h, 1)
{ this->mwindow = mwindow; this->gui = gui; }

TimeBarDownArrow::~TimeBarDownArrow() {}

int TimeBarDownArrow::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->next_label();
return 0;
}


