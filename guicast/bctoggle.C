#include "bcpixmap.h"
#include "bcresources.h"
#include "bctoggle.h"
#include "colors.h"
#include "fonts.h"


BC_Toggle::BC_Toggle(int x, int y, 
		VFrame **data, 
		int value, 
		char *caption,
		int bottom_justify, 
		int font,
		int color)
 : BC_SubWindow(x, y, 0, 0, -1)
{
	this->data = data;
	images[0] = images[1] = images[2] = images[3] = images[4] = 0;
	status = value ? TOGGLE_CHECKED : TOGGLE_UP;
	this->value = value;
	this->caption = caption;
	this->bottom_justify = bottom_justify;
	this->font = font;
	this->color = color;
}


BC_Toggle::~BC_Toggle()
{
	delete images[0];
	delete images[1];
	delete images[2];
	delete images[3];
	delete images[4];
}


int BC_Toggle::initialize()
{
// Get the image
	set_images(data);
	w = images[0]->get_w();
	h = images[0]->get_h();
	toggle_x = 0;
	toggle_y = 0;
	text_y = 0;
	text_x = w + 5;

// Expand subwindow for text
	if(has_caption())
	{
		text_w = get_text_width(MEDIUMFONT, caption);
		text_h = get_text_height(MEDIUMFONT);
		if(text_h > h)
		{
			toggle_y = (text_h - h) >> 1;
			h = text_h;
		}
		else
			text_y = (h - text_h) >> 1;

		if(bottom_justify)
		{
			text_y = h - text_h;
			text_line = h - get_text_descent(MEDIUMFONT);
		}
		else
			text_line = text_y + get_text_ascent(MEDIUMFONT);
		
		w = text_x + text_w;
	}

// Create the subwindow
	BC_SubWindow::initialize();

// Display the bitmap
	draw_face();
	return 0;
}

int BC_Toggle::set_images(VFrame **data)
{
	if(images)
	{
		delete images[0];
		delete images[1];
		delete images[2];
		delete images[3];
		delete images[4];
	}

	images[0] = new BC_Pixmap(top_level, data[0], PIXMAP_ALPHA);
	images[1] = new BC_Pixmap(top_level, data[1], PIXMAP_ALPHA);
	images[2] = new BC_Pixmap(top_level, data[2], PIXMAP_ALPHA);
	images[3] = new BC_Pixmap(top_level, data[3], PIXMAP_ALPHA);
	images[4] = new BC_Pixmap(top_level, data[4], PIXMAP_ALPHA);
	return 0;
}

int BC_Toggle::draw_face()
{
	draw_top_background(parent_window, 0, 0, get_w(), get_h());
	if(has_caption())
	{
		if(status == TOGGLE_UPHI || status == TOGGLE_DOWN || status == TOGGLE_CHECKEDHI)
		{
			set_color(LTGREY);
			draw_box(text_x, text_line - get_text_ascent(MEDIUMFONT), get_w() - text_x, get_text_height(MEDIUMFONT));
		}

		set_opaque();
		set_color(get_resources()->text_default);
		set_font(font);
		set_color(color);
		draw_text(text_x, text_line, caption);
	}

	draw_pixmap(images[status]);
	flash();
	return 0;
}


int BC_Toggle::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->tooltip_delay &&
		tooltip_text[0] != 0 &&
		(status == TOGGLE_UPHI || status == TOGGLE_CHECKEDHI) &&
		!tooltip_done)
	{
		show_tooltip();
		tooltip_done = 1;
		return 1;
	}
	return 0;
}

int BC_Toggle::cursor_enter_event()
{
	if(top_level->event_win == win)
	{
		tooltip_done = 0;
		if(top_level->button_down)
			status = TOGGLE_DOWN;
		else
			status = value ? TOGGLE_CHECKEDHI : TOGGLE_UPHI;
		draw_face();
	}
	return 0;
}

int BC_Toggle::cursor_leave_event()
{
	hide_tooltip();
	if(!value)
	{
		status = TOGGLE_UP;
		draw_face();
	}
	else
	{
		status = TOGGLE_CHECKED;
		draw_face();
	}
	return 0;
}

int BC_Toggle::button_press_event()
{
	hide_tooltip();
	if(top_level->event_win == win)
	{
		status = TOGGLE_DOWN;
		draw_face();
		return 1;
	}
	return 0;
}

int BC_Toggle::button_release_event()
{
	hide_tooltip();
	if(top_level->event_win == win && status == TOGGLE_DOWN)
	{
		if(!value)
		{
			status = TOGGLE_CHECKEDHI;
			value = 1;
			draw_face();
			return handle_event();
		}
		else
		{
			status = TOGGLE_UPHI;
			value = 0;
			draw_face();
			return handle_event();
		}
	}
	return 0;
}

int BC_Toggle::cursor_motion_event()
{
	if(top_level->button_down && top_level->event_win == win && !cursor_inside())
	{
		if(status == TOGGLE_DOWN)
		{
			status = TOGGLE_UP;
			draw_face();
		}
		else
		if(status == TOGGLE_UPHI)
		{
			status = TOGGLE_CHECKEDHI;
			draw_face();
		}
	}
	return 0;
}

int BC_Toggle::get_value()
{
	return value;
}

int BC_Toggle::set_value(int value)
{
	this->value = value;
	if(value) 
	switch(status)
	{
		case TOGGLE_UP:
			status = TOGGLE_CHECKED;
			break;
		case TOGGLE_UPHI:
			status = TOGGLE_CHECKEDHI;
			break;
	}
	else
	switch(status)
	{
		case TOGGLE_CHECKED:
			status = TOGGLE_UP;
			break;
		case TOGGLE_CHECKEDHI:
			status = TOGGLE_UPHI;
			break;
	}
	draw_face();
	return 0;
}

int BC_Toggle::update(int value)
{
	return set_value(value);
}

void BC_Toggle::reposition_window(int x, int y)
{
	BC_WindowBase::reposition_window(x, y);
	draw_face();
}


int BC_Toggle::has_caption()
{
	return (caption != 0 && caption[0] != 0);
}

BC_Radial::BC_Radial(int x, 
	int y, 
	int value, 
	char *caption, 
	int font,
	int color)
 : BC_Toggle(x, 
 	y, 
	BC_WindowBase::get_resources()->radial_images, 
	value, 
	caption, 
	0, 
	font,
	color)
{
}

BC_CheckBox::BC_CheckBox(int x, 
	int y, 
	int value, 
	char *caption, 
	int font,
	int color)
 : BC_Toggle(x, 
 	y, 
	BC_WindowBase::get_resources()->checkbox_images, 
	value, 
	caption, 
	1, 
	font,
	color)
{
	this->value = 0;
}

BC_CheckBox::BC_CheckBox(int x, 
	int y, 
	int *value, 
	char *caption, 
	int font,
	int color)
 : BC_Toggle(x, 
 	y, 
	BC_WindowBase::get_resources()->checkbox_images, 
	*value, 
	caption, 
	1, 
	font,
	color)
{
	this->value = value;
}

int BC_CheckBox::handle_event()
{
	*value = get_value();
	return 1;
}




BC_Label::BC_Label(int x, 
	int y, 
	int value, 
	int font,
	int color)
 : BC_Toggle(x, 
 	y, 
	BC_WindowBase::get_resources()->label_images, 
	value, 
	"", 
	0, 
	font,
	color)
{
}
