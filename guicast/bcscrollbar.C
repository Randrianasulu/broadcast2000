#include "bcresources.h"
#include "bcscrollbar.h"
#include "colors.h"

BC_ScrollBar::BC_ScrollBar(int x, 
	int y, 
	int orientation, 
	int pixels, 
	long length, 
	long position, 
	long handlelength)
 : BC_SubWindow(x, y, 0, 0, -1)
{
	this->length = length;
	this->position = position;
	this->handlelength = handlelength;
	this->selection_status = 0;
	this->highlight_status = 0;
	this->orientation = orientation;
	this->pixels = pixels;
	handle_pixel = 0;
	handle_pixels = 0;
	bound_to = 0;
	repeat_count = 0;
}

BC_ScrollBar::~BC_ScrollBar()
{
}

int BC_ScrollBar::initialize()
{
//printf("BC_ScrollBar::initialize 1\n");
	switch(orientation)
	{
		case SCROLL_HORIZ:
			w = pixels;
			h = SCROLL_SPAN;
			break;

		case SCROLL_VERT:
			w = SCROLL_SPAN;
			h = pixels;
			break;
	}
//printf("BC_ScrollBar::initialize 1\n");

	BC_SubWindow::initialize();
//printf("BC_ScrollBar::initialize 1\n");
	draw();
//printf("BC_ScrollBar::initialize 2\n");
	return 0;
}

void BC_ScrollBar::draw()
{
	get_handle_dimensions();
	switch(orientation)
	{
		case SCROLL_HORIZ:
			if(get_w() < SCROLL_SPAN * 2 + 5)
			{
				draw_colored_box(0, 0, get_w(), get_h(), 0, 0);
			}
			else
			{
// background box
				draw_colored_box(0, 0, get_w(), get_h(), 1, 0);
// handle
				draw_colored_box(handle_pixel, 
					SCROLL_MARGIN, 
					handle_pixels, 
					SCROLL_SPAN - SCROLL_MARGIN * 2, 
					selection_status == SCROLL_HANDLE, 
					highlight_status == SCROLL_HANDLE && selection_status != SCROLL_HANDLE);

// back arrow
				if(selection_status == SCROLL_BACKARROW)
					draw_triangle_left(SCROLL_MARGIN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_shadow, 
						BLACK, 
						get_resources()->button_down, 
						get_resources()->button_down, 
						get_resources()->button_light);
				else
				if(highlight_status == SCROLL_BACKARROW)
					draw_triangle_left(SCROLL_MARGIN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_light, 
						get_resources()->button_highlighted, 
						get_resources()->button_highlighted, 
						get_resources()->button_down, 
						BLACK);
				else
					draw_triangle_left(SCROLL_MARGIN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_light, 
						get_resources()->button_up, 
						get_resources()->button_up, 
						get_resources()->button_down, 
						BLACK);

// forward arrow
				if(selection_status == SCROLL_FWDARROW)
					draw_triangle_right(get_w() - SCROLL_SPAN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_shadow, 
						BLACK, 
						get_resources()->button_down, 
						get_resources()->button_down, 
						get_resources()->button_light);
				else
				if(highlight_status == SCROLL_FWDARROW)
					draw_triangle_right(get_w() - SCROLL_SPAN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_light, 
						get_resources()->button_highlighted, 
						get_resources()->button_highlighted, 
						get_resources()->button_down, 
						BLACK);
				else
					draw_triangle_right(get_w() - SCROLL_SPAN, 
						SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						get_resources()->button_light, 
						get_resources()->button_up, 
						get_resources()->button_up, 
						get_resources()->button_down, 
						BLACK);
			}
			break;
		
		case SCROLL_VERT:
			if(get_h() < SCROLL_SPAN * 2 + 5)
			{
				draw_colored_box(0, 0, get_w(), get_h(), 0, 0);
			}
			else
			{
// background box
				draw_colored_box(0, 0, get_w(), get_h(), 1, 0);
// handle
				draw_colored_box(SCROLL_MARGIN, 
					handle_pixel, 
					SCROLL_SPAN - SCROLL_MARGIN * 2, 
					handle_pixels, 
					selection_status == SCROLL_HANDLE, 
					highlight_status == SCROLL_HANDLE && selection_status != SCROLL_HANDLE);

// back arrow
				if(selection_status == SCROLL_BACKARROW)
					draw_triangle_up(SCROLL_MARGIN, SCROLL_MARGIN, SCROLL_SPAN - SCROLL_MARGIN * 2, SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_shadow, 
						BLACK, 
						get_resources()->button_down, 
						get_resources()->button_down, 
						get_resources()->button_light);
				else
				if(highlight_status == SCROLL_BACKARROW)
					draw_triangle_up(SCROLL_MARGIN, SCROLL_MARGIN, SCROLL_SPAN - SCROLL_MARGIN * 2, SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_light, 
						get_resources()->button_highlighted, 
						get_resources()->button_highlighted, 
						get_resources()->button_down, 
						BLACK);
				else
					draw_triangle_up(SCROLL_MARGIN, SCROLL_MARGIN, SCROLL_SPAN - SCROLL_MARGIN * 2, SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_light, 
						get_resources()->button_up, 
						get_resources()->button_up, 
						get_resources()->button_down, 
						BLACK);

// forward arrow
				if(selection_status == SCROLL_FWDARROW)
					draw_triangle_down(SCROLL_MARGIN, 
						get_h() - SCROLL_SPAN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_shadow, 
						BLACK, 
						get_resources()->button_down, 
						get_resources()->button_down, 
						get_resources()->button_light);
				else
				if(highlight_status == SCROLL_FWDARROW)
					draw_triangle_down(SCROLL_MARGIN, 
						get_h() - SCROLL_SPAN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_light, 
						get_resources()->button_highlighted, 
						get_resources()->button_highlighted, 
						get_resources()->button_down, 
						BLACK);
				else
					draw_triangle_down(SCROLL_MARGIN, 
						get_h() - SCROLL_SPAN, 
						SCROLL_SPAN - SCROLL_MARGIN * 2, 
						SCROLL_SPAN - SCROLL_MARGIN, 
						get_resources()->button_light, 
						get_resources()->button_up, 
						get_resources()->button_up, 
						get_resources()->button_down, 
						BLACK);
			}
			break;
	}
	flash();
}

void BC_ScrollBar::get_handle_dimensions()
{
	double total_pixels = pixels - SCROLL_SPAN * 2;
//printf("BC_ScrollBar::get_handle_dimensions %ld\n", length);
	if(length > 0)
	{
		handle_pixels = (long)((double)handlelength / length * total_pixels + .5);
		if(handle_pixels < MINHANDLE) handle_pixels = MINHANDLE;
		handle_pixel = (long)((double)position / length * total_pixels + .5) + SCROLL_SPAN;
		if(handle_pixel > pixels - SCROLL_SPAN - handle_pixels)
			handle_pixel = pixels - SCROLL_SPAN - handle_pixels;
	}
	else
	{
		handle_pixels = total_pixels - SCROLL_SPAN * 2;
		handle_pixel = SCROLL_SPAN;
	}
}

int BC_ScrollBar::cursor_enter_event()
{
	if(top_level->event_win == win)
	{
		if(!highlight_status)
		{
			highlight_status = get_cursor_zone(top_level->cursor_x, top_level->cursor_y);
			draw();
			flash();
		}
		return 1;
	}
	return 0;
}

int BC_ScrollBar::cursor_leave_event()
{
	if(highlight_status)
	{
		highlight_status = 0;
		draw();
		flash();
	}
	return 0;
}

int BC_ScrollBar::cursor_motion_event()
{
	if(top_level->event_win == win)
	{
		if(highlight_status && !selection_status)
		{
			int new_highlight_status = get_cursor_zone(top_level->cursor_x, top_level->cursor_y);
			if(new_highlight_status != highlight_status)
			{
				highlight_status = new_highlight_status;
				draw();
				flash();
			}
		}
		else
		if(selection_status == SCROLL_HANDLE)
		{
			
			double total_pixels = pixels - SCROLL_SPAN * 2;
			long cursor_pixel = (orientation == SCROLL_HORIZ) ? top_level->cursor_x : top_level->cursor_y;
			long new_position = (long)((double)(cursor_pixel - min_pixel) / total_pixels * length);
			if(new_position > length - handlelength) new_position = length - handlelength;
			else
			if(new_position < 0) new_position = 0;
			if(new_position != position)
			{
				position = new_position;
				draw();
				flash();
				handle_event();
			}
		}
		return 1;
	}
	return 0;
}

int BC_ScrollBar::button_press_event()
{
	if(top_level->event_win == win)
	{
		if(!bound_to)
		{
			top_level->deactivate();
			activate();
		}
		selection_status = get_cursor_zone(top_level->cursor_x, top_level->cursor_y);
		if(selection_status == SCROLL_HANDLE)
		{
			double total_pixels = pixels - SCROLL_SPAN * 2;
			long cursor_pixel = (orientation == SCROLL_HORIZ) ? top_level->cursor_x : top_level->cursor_y;
			min_pixel = cursor_pixel - (long)((double)position / length * total_pixels + .5);
			max_pixel = (int)(cursor_pixel + total_pixels);
			draw();
			flash();
		}
		else
		if(selection_status)
		{
			top_level->set_repeat(top_level->get_resources()->scroll_repeat);
			repeat_count = 0;
			repeat_event(top_level->get_resources()->scroll_repeat);
		}
		return 1;
	}
	return 0;
}

int BC_ScrollBar::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->scroll_repeat && 
		selection_status)
	{
		repeat_count++;
		if(repeat_count == 2) return 0;
		long new_position = position;
		switch(selection_status)
		{
			case SCROLL_BACKPAGE:
				new_position -= handlelength;
				break;
			case SCROLL_FWDPAGE:
				new_position += handlelength;
				break;
			case SCROLL_BACKARROW:
				new_position -= handlelength / 10;
				break;
			case SCROLL_FWDARROW:
				new_position += handlelength / 10;
				break;
		}
		if(new_position > length - handlelength) new_position = length - handlelength;
		else
		if(new_position < 0) new_position = 0;
		if(new_position != position)
		{
			position = new_position;
			draw();
			flash();
			handle_event();
		}
		return 1;
	}
	return 0;
}

int BC_ScrollBar::button_release_event()
{
	if(selection_status)
	{
		if(selection_status != SCROLL_HANDLE)
			top_level->unset_repeat(top_level->get_resources()->scroll_repeat);

		selection_status = 0;
		draw();
		flash();
		return 1;
	}
	return 0;
}

int BC_ScrollBar::get_cursor_zone(int cursor_x, int cursor_y)
{
	if(orientation == SCROLL_VERT)
	{
		cursor_x ^= cursor_y;
		cursor_y ^= cursor_x;
		cursor_x ^= cursor_y;
	}

	if(cursor_x >= pixels - SCROLL_SPAN)
		return SCROLL_FWDARROW;
	else
	if(cursor_x >= SCROLL_SPAN)
	{
		if(cursor_x > handle_pixel + handle_pixels)
			return SCROLL_FWDPAGE;
		else
		if(cursor_x >= handle_pixel)
			return SCROLL_HANDLE;
		else
			return SCROLL_BACKPAGE;
	}
	else
		return SCROLL_BACKARROW;
	
	return 0;
}

int BC_ScrollBar::activate()
{
	top_level->active_subwindow = this;
	return 0;
}

long BC_ScrollBar::get_value()
{
	return position;
}

long BC_ScrollBar::get_position()
{
	return position;
}

long BC_ScrollBar::get_length()
{
	return length;
}

int BC_ScrollBar::in_use()
{
	return selection_status != 0;
}

long BC_ScrollBar::get_handlelength()
{
	return handlelength;
}

int BC_ScrollBar::update_value(long value)
{
	this->position = value;
	draw();
	return 0;
}

int BC_ScrollBar::update_length(long length, long position, long handlelength)
{
	this->length = length;
	this->position = position;
	this->handlelength = handlelength;
	draw();
	return 0;
}

int BC_ScrollBar::reposition_window(int x, int y, int w, int h)
{
	BC_WindowBase::reposition_window(x, y, w, h);
	this->pixels = orientation == SCROLL_VERT ? h : w;
	draw();
	return 0;
}

