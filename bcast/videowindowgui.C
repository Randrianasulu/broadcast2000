#include <string.h>
#include "buttonbar.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "videowindow.h"
#include "videowindowgui.h"



#define CROPHANDLE_W 10
#define CROPHANDLE_H 10

VideoWindowGUI::VideoWindowGUI(VideoWindow *thread, int w, int h)
 : BC_Window("", MEGREY, ICONNAME ": Video out", 
 	w , h , 10, 10, 1, !thread->video_visible)
{
	this->thread = thread;
}

VideoWindowGUI::~VideoWindowGUI()
{
}

int VideoWindowGUI::create_objects()
{
	add_tool(canvas = new VideoWindowCanvas(this, get_w(), get_h()));
	update_title();
return 0;
}


int VideoWindowGUI::keypress_event()
{
	int result = 0;
	if(shift_down()) return  0;
	if(get_keypress() == 'w')
	{
		thread->hide_window();
		return 1;
	}
	else
	if(get_keypress() == 'l')
	{
		thread->mwindow->gui->lock_window();
		thread->mwindow->toggle_label();
		thread->mwindow->gui->unlock_window();
	}

// make it look like mwindow is handling the event
	thread->mwindow->gui->lock_window();
	unlock_window();
	result = thread->mwindow->gui->buttonbar->transport_keys(get_keypress());
	lock_window();   // so video device can clean this up
	thread->mwindow->gui->unlock_window();
	return result;
return 0;
}

int VideoWindowGUI::resize_event(int w, int h)
{
	int output_w = thread->mwindow->output_w;
	int output_h = thread->mwindow->output_h;
	int new_w, new_h, full_w, full_h;

	new_w = w;
	new_h = h;
	thread->get_full_sizes(full_w, full_h);

	if(labs(full_w - new_w) < 50)
	{
		new_w = full_w;
		new_h = full_h;
	}
	else
		thread->fix_size(new_w, new_h, w, thread->mwindow->get_aspect_ratio());

	if(new_w < 10) new_w = 10;
	if(new_h < 10) new_h = 10;
	w = thread->video_window_w = new_w;
	h = new_h;

	resize_window(w, h);
	canvas->set_size(0, 0, w, h);
	update_title();
	if(thread->video_cropping) canvas->draw_crop_box();
return 0;
}

int VideoWindowGUI::update_title()
{
	char string[1024];

	if(thread->mwindow->get_aspect_ratio() > (float)thread->mwindow->output_w / thread->mwindow->output_h)
	{
		sprintf(string, ICONNAME ": Video out %d%%", 
			(int)((float)thread->video_window_w / (thread->mwindow->output_h * thread->mwindow->get_aspect_ratio()) * 100 + 0.5));
	}
	else
	{
		sprintf(string, ICONNAME ": Video out %d%%", 
			(int)((float)thread->video_window_w / thread->mwindow->output_w * 100 + 0.5));
	}

	set_title(string);
return 0;
}

int VideoWindowGUI::close_event()
{
	thread->hide_window();
return 0;
}


VideoWindowCanvas::VideoWindowCanvas(VideoWindowGUI *gui, int w, int h)
 : BC_Canvas(0, 0, w, h, BLACK)
{
	this->gui = gui;
	corner_selected = 0;
	button_down = 0;
}

VideoWindowCanvas::~VideoWindowCanvas()
{
}

int VideoWindowCanvas::button_press()
{
	if(gui->thread->video_cropping)
	{
		int x = get_cursor_x();
		int y = get_cursor_y();
		button_down = 1;

		if(x > gui->x1 && y > gui->y1 && x < gui->x1 + CROPHANDLE_W && y < gui->y1 + CROPHANDLE_H)
		{
			corner_selected = 1;
			gui->x_offset = x - gui->x1;
			gui->y_offset = y - gui->y1;
		}
		if(x > gui->x1 && y > gui->y2 - CROPHANDLE_H && x < gui->x1 + CROPHANDLE_W && y < gui->y2)
		{
			corner_selected = 2;
			gui->x_offset = x - gui->x1;
			gui->y_offset = y - gui->y2;
		}
		if(x > gui->x2 - CROPHANDLE_W && y > gui->y2 - CROPHANDLE_H && x < gui->x2 && y < gui->y2)
		{
			corner_selected = 3;
			gui->x_offset = x - gui->x2;
			gui->y_offset = y - gui->y2;
		}
		if(x > gui->x2 - CROPHANDLE_W && y > gui->y1 && x < gui->x2 && y < gui->y1 + CROPHANDLE_H)
		{
			corner_selected = 4;
			gui->x_offset = x - gui->x2;
			gui->y_offset = y - gui->y1;
		}
	}
return 0;
}

int VideoWindowCanvas::button_release()
{
	if(gui->thread->video_cropping && button_down)
	{
		button_down = 0;
		corner_selected = 0;
	}
return 0;
}

int VideoWindowCanvas::cursor_motion()
{
	if(button_down && gui->thread->video_cropping && corner_selected)
	{
		int x = get_cursor_x();
		int y = get_cursor_y();
		draw_crop_box();

		switch(corner_selected)
		{
			case 1:
				gui->x1 = x - gui->x_offset;  gui->y1 = y - gui->y_offset;
				break;
			case 2:
				gui->x1 = x - gui->x_offset;  gui->y2 = y - gui->y_offset;
				break;
			case 3:
				gui->x2 = x - gui->x_offset;  gui->y2 = y - gui->y_offset;
				break;
			case 4:
				gui->x2 = x - gui->x_offset;  gui->y1 = y - gui->y_offset;
				break;
		};

		if(gui->x1 < 0) gui->x1 = 0;
		if(gui->y1 < 0) gui->y1 = 0;
		if(gui->x1 > get_w()) gui->x1 = get_w();
		if(gui->y1 > get_h()) gui->y1 = get_h();
		if(gui->x2 < 0) gui->x2 = 0;
		if(gui->y2 < 0) gui->y2 = 0;
		if(gui->x2 > get_w()) gui->x2 = get_w();
		if(gui->y2 > get_h()) gui->y2 = get_h();
		draw_crop_box();
		flash();
	}
return 0;
}

int VideoWindowCanvas::draw_crop_box()
{
	int w = gui->x2 - gui->x1;
	int h = gui->y2 - gui->y1;

	set_inverse();
	set_color(WHITE);
	draw_box(gui->x1 + 1, gui->y1 + 1, CROPHANDLE_W - 1, CROPHANDLE_H - 1);
	draw_box(gui->x1 + 1, gui->y2 - CROPHANDLE_H, CROPHANDLE_W - 1, CROPHANDLE_H);
	draw_box(gui->x2 - CROPHANDLE_W, gui->y2 - CROPHANDLE_H, CROPHANDLE_W, CROPHANDLE_H);
	draw_box(gui->x2 - CROPHANDLE_W, gui->y1 + 1, CROPHANDLE_W, CROPHANDLE_H - 1);
	draw_rectangle(gui->x1, gui->y1, w, h);
	set_opaque();
return 0;
}

