#include "bcbutton.h"
#include "bcresources.h"
#include "bcpixmap.h"
#include "colors.h"
#include "fonts.h"
#include "keys.h"
#include "vframe.h"

#include <string.h>

#define BUTTON_UP 0
#define BUTTON_UPHI 1
#define BUTTON_DOWNHI 2

BC_Button::BC_Button(int x, int y, 
	VFrame **data)
 : BC_SubWindow(x, y, 0, 0, -1)
{
	this->data = data;
	for(int i = 0; i < 9; i++) images[i] = 0;
	status = BUTTON_UP;
}


BC_Button::~BC_Button()
{
	for(int i = 0; i < 9; i++) if(images[i]) delete images[i];
}



int BC_Button::initialize()
{
// Get the image
	set_images(data);

// Create the subwindow
	BC_SubWindow::initialize();

// Display the bitmap
	draw_face();
	return 0;
}

int BC_Button::reposition_window(int x, int y)
{
	BC_WindowBase::reposition_window(x, y);
	return 0;
}


int BC_Button::update_bitmaps(VFrame **data)
{
	this->data = data;
	set_images(data);
	draw_top_background(parent_window, 0, 0, w, h);
	draw_face();
	return 0;
}

int BC_Button::set_images(VFrame **data)
{
	for(int i = 0; i < 3; i++)
	{
		if(images[i]) delete images[i];
		images[i] = new BC_Pixmap(parent_window, data[i], PIXMAP_ALPHA);
	}
	w = images[BUTTON_UP]->get_w();
	h = images[BUTTON_UP]->get_h();
	return 0;
}

int BC_Button::draw_face()
{
	draw_top_background(parent_window, 0, 0, w, h);
	images[status]->write_drawable(pixmap, 
			0, 
			0,
			w,
			h,
			0,
			0);
	flash();
	return 0;
}

int BC_Button::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->tooltip_delay &&
		tooltip_text[0] != 0 &&
		status == BUTTON_UPHI &&
		!tooltip_done)
	{
		show_tooltip();
		tooltip_done = 1;
		return 1;
	}
	return 0;
}

int BC_Button::cursor_enter_event()
{
	if(top_level->event_win == win)
	{
		tooltip_done = 0;
		if(top_level->button_down)
		{
			status = BUTTON_DOWNHI;
		}
		else
		if(status == BUTTON_UP) status = BUTTON_UPHI;
		draw_face();
	}
	return 0;
}

int BC_Button::cursor_leave_event()
{
	if(status == BUTTON_UPHI)
	{
		status = BUTTON_UP;
		draw_face();
		hide_tooltip();
	}
	return 0;
}

int BC_Button::button_press_event()
{
	if(top_level->event_win == win)
	{
		top_level->hide_tooltip();
		if(status == BUTTON_UPHI || status == BUTTON_UP) status = BUTTON_DOWNHI;
		draw_face();
		return 1;
	}
	return 0;
}

int BC_Button::button_release_event()
{
	if(top_level->event_win == win)
	{
		hide_tooltip();
		if(status == BUTTON_DOWNHI) 
		{
			status = BUTTON_UPHI;
			draw_face();

			if(cursor_inside())
			{
				handle_event();
				return 1;
			}
		}
	}
	return 0;
}

int BC_Button::cursor_motion_event()
{
	if(top_level->button_down && top_level->event_win == win && 
		status == BUTTON_DOWNHI && !cursor_inside())
	{
		status = BUTTON_UP;
		draw_face();
	}
	return 0;
}


BC_OKButton::BC_OKButton(int x, int y)
 : BC_Button(x, y, 
 	BC_WindowBase::get_resources()->ok_images)
{
}

BC_OKButton::BC_OKButton(BC_WindowBase *parent_window)
 : BC_Button(10, 
 	parent_window->get_h() - BC_WindowBase::get_resources()->cancel_images[0]->get_h() - 10, 
 	BC_WindowBase::get_resources()->ok_images)
{
}

int BC_OKButton::handle_event()
{
	get_top_level()->set_done(0);
	return 0;
}

int BC_OKButton::keypress_event()
{
	if(get_keypress() == RETURN) return handle_event();
	return 0;
}

BC_CancelButton::BC_CancelButton(int x, int y)
 : BC_Button(x, y, 
 	BC_WindowBase::get_resources()->cancel_images)
{
}

BC_CancelButton::BC_CancelButton(BC_WindowBase *parent_window)
 : BC_Button(parent_window->get_w() - BC_WindowBase::get_resources()->cancel_images[0]->get_w() - 10, 
 	parent_window->get_h() - BC_WindowBase::get_resources()->cancel_images[0]->get_h() - 10, 
 	BC_WindowBase::get_resources()->cancel_images)
{
}

int BC_CancelButton::handle_event()
{
	get_top_level()->set_done(1);
	return 1;
}

int BC_CancelButton::keypress_event()
{
	if(get_keypress() == ESC) return handle_event();
	return 0;
}




#define LEFT_DN  0
#define LEFT_HI  1
#define LEFT_UP  2
#define MID_DN   3
#define MID_HI   4
#define MID_UP   5
#define RIGHT_DN 6
#define RIGHT_HI 7
#define RIGHT_UP 8

BC_GenericButton::BC_GenericButton(int x, int y, char *text)
 : BC_Button(x, y, BC_WindowBase::get_resources()->generic_button_images)
{
	strcpy(this->text, text);
}

int BC_GenericButton::set_images(VFrame **data)
{
	for(int i = 0; i < 9; i++)
	{
		if(images[i]) delete images[i];
		images[i] = new BC_Pixmap(parent_window, data[i], PIXMAP_ALPHA);
	}
	w = get_text_width(MEDIUMFONT, text) + images[LEFT_UP]->get_w() + images[RIGHT_UP]->get_w();
	h = images[BUTTON_UP]->get_h();
	return 0;
}

int BC_GenericButton::draw_face()
{
	draw_top_background(parent_window, 0, 0, w, h);

	for(int i = 0; i < get_w(); )
	{
// Get image to draw
		int image_number = 0;
		int left_boundary = images[LEFT_UP]->get_w_fixed();
		int right_boundary = get_w() - images[RIGHT_UP]->get_w_fixed();

		if(i < left_boundary)
		{
			image_number = LEFT_DN;
		}
		else
		if(i < right_boundary)
		{
			image_number = MID_DN;
		}
		else
			image_number = RIGHT_DN;

		if(status == BUTTON_UP) image_number += 2;
		else
		if(status == BUTTON_UPHI) image_number += 1;

		int output_w = images[image_number]->get_w_fixed();

		if(i < left_boundary)
		{
			if(i + output_w > left_boundary) output_w = left_boundary - i;
		}
		else
		if(i < right_boundary)
		{
			if(i + output_w > right_boundary) output_w = right_boundary - i;
		}
		else
			if(i + output_w > get_w()) output_w = get_w() - i;

		images[image_number]->write_drawable(pixmap, 
				i, 
				0,
				output_w,
				h,
				0,
				0);

		set_color(BLACK);
		set_font(MEDIUMFONT);
		draw_center_text(get_w() / 2, 
			(int)((float)get_h() / 2 + get_text_ascent(MEDIUMFONT) / 2 - 2), 
			text);

		i += output_w;
	}

	flash();
	return 0;
}
