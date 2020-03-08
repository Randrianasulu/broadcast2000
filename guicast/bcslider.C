#include "bcpixmap.h"
#include "bcresources.h"
#include "bcslider.h"
#include "colors.h"
#include "fonts.h"
#include "keys.h"

#include <ctype.h>
#include <string.h>

#define SLIDER_UP 0
#define SLIDER_UPHI 1
#define SLIDER_DN 2
#define LEFTSLIDER 3
#define MIDSLIDER 4
#define RIGHTSLIDER 5
#define TOTAL_IMAGES 6



BC_Slider::BC_Slider(int x, 
		int y, 
		int pixels, 
		int pointer_motion_range,  
		VFrame **data, 
		int show_number, 
		int vertical,
		int use_caption)
 : BC_SubWindow(x, y, 0, 0, -1)
{
	this->data = data;
	this->show_number = show_number;
	this->vertical = vertical;
	this->pointer_motion_range = pointer_motion_range;
	this->pixels = pixels;
	this->button_pixel = button_pixel;
	this->use_caption = use_caption;
	
	status = SLIDER_UP;
	images = new BC_Pixmap*[TOTAL_IMAGES];
	for(int i = 0; i < TOTAL_IMAGES; i++) images[i] = 0;
	button_down = 0;
}

BC_Slider::~BC_Slider()
{
	delete [] images;
}

int BC_Slider::initialize()
{
	if(!data)
	{
		this->data = vertical ? 
			BC_WindowBase::get_resources()->vertical_slider : 
			BC_WindowBase::get_resources()->horizontal_slider;
	}

	set_images(data);

	if(vertical)
	{
		w = images[SLIDER_UP]->get_w();
		h = pixels;
	}
	else
	{
		h = images[SLIDER_UP]->get_h();
		w = pixels;
	}

	text_height = get_text_height(SMALLFONT);
	button_pixel = value_to_pixel();

	BC_SubWindow::initialize();
	draw_face();
	return 0;
}

int BC_Slider::draw_face()
{
// Clear background
	draw_top_background(parent_window, 0, 0, get_w(), get_h());


//	if(button_pixel > get_button_pixels()) button_pixel = get_button_pixels();
	if(vertical)
	{
		for(int y = images[LEFTSLIDER]->get_h(); 
			y < h - images[RIGHTSLIDER]->get_h() - images[MIDSLIDER]->get_h(); 
			y += images[MIDSLIDER]->get_h())
		{
			draw_pixmap(images[MIDSLIDER], 0, y);
		}

		draw_pixmap(images[MIDSLIDER], 0, h - images[RIGHTSLIDER]->get_h() - images[MIDSLIDER]->get_h());
		draw_pixmap(images[LEFTSLIDER], 0, 0);
		draw_pixmap(images[RIGHTSLIDER], 0, h - images[RIGHTSLIDER]->get_h());
		draw_pixmap(images[status], 0, button_pixel);
		set_color(RED);
		set_font(SMALLFONT);
		draw_text(0, h, get_caption());
	}
	else
	{
		int y = get_h() / 2 - images[LEFTSLIDER]->get_h() / 2;
		draw_pixmap(images[LEFTSLIDER], 0, y);
		for(int x = images[LEFTSLIDER]->get_w() - 1; 
			x < w - images[RIGHTSLIDER]->get_w(); )
		{
			int dest_w;
			if(x + images[MIDSLIDER]->get_w() < w - images[RIGHTSLIDER]->get_w())
				dest_w = images[MIDSLIDER]->get_w();
			else
				dest_w = w - images[RIGHTSLIDER]->get_w() - x + 1;
			draw_pixmap(images[MIDSLIDER], x, y, dest_w);
			x += dest_w - 1;
		}
		draw_pixmap(images[RIGHTSLIDER], w - images[RIGHTSLIDER]->get_w(), y);

		draw_pixmap(images[status], button_pixel, 0);
		set_color(RED);
		set_font(SMALLFONT);
		if(use_caption) draw_text(0, h, get_caption());
	}

	flash();
	return 0;
}

int BC_Slider::set_images(VFrame **data)
{
	for(int i = 0; i < TOTAL_IMAGES; i++)
	{
		if(images[i]) delete images[i];
		images[i] = new BC_Pixmap(parent_window, data[i], PIXMAP_ALPHA);
	}
	return 0;
}

int BC_Slider::get_button_pixels()
{
	return vertical ? images[SLIDER_UP]->get_h() : images[SLIDER_UP]->get_w();
}

void BC_Slider::show_value_tooltip()
{
	set_tooltip(get_caption());
	show_tooltip(50);
	keypress_tooltip_timer = 2000;
}


int BC_Slider::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->tooltip_delay)
	{
		if(tooltip_on)
		{
			if(keypress_tooltip_timer > 0)
			{
				keypress_tooltip_timer -= get_resources()->tooltip_delay;
			}
			else
			if(status != SLIDER_UPHI && status != SLIDER_DN)
			{
				hide_tooltip();
			}
		}
		else
		if(status == SLIDER_UPHI)
		{
			if(!tooltip_text[0] || isdigit(tooltip_text[0]))
			{
				set_tooltip(get_caption());
				show_tooltip(50);
			}
			else
				show_tooltip();
			tooltip_done = 1;
			return 1;
		}
	}
	return 0;
}


int BC_Slider::keypress_event()
{
	int result = 0;
	switch(get_keypress())
	{
		case UP:
			increase_value();
			result = 1;
			break;
		case DOWN:
			decrease_value();
			result = 1;
			break;
		case LEFT:
			decrease_value();
			result = 1;
			break;
		case RIGHT:
			increase_value();
			result = 1;
			break;
	}

	if(result)
	{
		show_value_tooltip();
		draw_face();
		handle_event();
	}
	return result;
}

int BC_Slider::cursor_enter_event()
{
	if(top_level->event_win == win && status == SLIDER_UP)
	{
		tooltip_done = 0;
		status = SLIDER_UPHI;
		draw_face();
	}
	return 0;
}

int BC_Slider::cursor_leave_event()
{
	if(status == SLIDER_UPHI)
	{
		status = SLIDER_UP;
		draw_face();
		hide_tooltip();
	}
	return 0;
}

int BC_Slider::button_press_event()
{
	int result = 0;
	if(top_level->event_win == win)
	{
		if(!tooltip_on) top_level->hide_tooltip();
		if(status == SLIDER_UPHI)
		{
			button_down = 1;
			status = SLIDER_DN;
			draw_face();
			init_selection(top_level->cursor_x, top_level->cursor_y);
			top_level->deactivate();
			top_level->active_subwindow = this;
			show_value_tooltip();
			result = 1;
		}
	}
	return result;
}

int BC_Slider::button_release_event()
{
	if(button_down) 
	{
		button_down = 0;
		if(cursor_inside()) 
			status = SLIDER_UPHI;
		else
		{
			status = SLIDER_UP;
			top_level->hide_tooltip();
		}
		draw_face();
		return 1;
	}
	return 0;
}

int BC_Slider::cursor_motion_event()
{
	if(button_down)
	{
		int old_pixel = button_pixel;
		int result = update_selection(top_level->cursor_x, top_level->cursor_y);
		if(button_pixel != old_pixel) draw_face();
		if(result) 
		{
			set_tooltip(get_caption());
			handle_event();
		}
		return 1;
	}
	return 0;
}

int BC_Slider::reposition_window(int x, int y, int w, int h)
{
	BC_WindowBase::reposition_window(x, y, w, h);
	draw_face();
	return 0;
}


int BC_Slider::get_pointer_motion_range()
{
	return pointer_motion_range;
}





BC_ISlider::BC_ISlider(int x, 
			int y,
			int vertical,
			int pixels, 
			int pointer_motion_range, 
			long minvalue, 
			long maxvalue, 
			long value,
			int use_caption,
			VFrame **data,
			int *output)
 : BC_Slider(x, 
		y, 
		pixels, 
		pointer_motion_range,  
		data,
		1, 
		vertical,
		use_caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
	this->output = output;
}

int BC_ISlider::value_to_pixel()
{
	if(maxvalue == minvalue) return 0;
	else
	{
		if(vertical)
			return (int)((1.0 - (double)(value - minvalue) / (maxvalue - minvalue)) * 
				(get_h() - get_button_pixels()));
		else
			return (int)((double)(value - minvalue) / (maxvalue - minvalue) * 
				(get_w() - get_button_pixels()));
	}
}

int BC_ISlider::update(long value)
{
	this->value = value;
	int old_pixel = button_pixel;
	button_pixel = value_to_pixel();
	if(button_pixel != old_pixel) draw_face();
	return 0;
}

int BC_ISlider::update(int pointer_motion_range, long value, long minvalue, long maxvalue)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
	this->pointer_motion_range = pointer_motion_range;
	return 0;
}


long BC_ISlider::get_value()
{
	return value;
}

long BC_ISlider::get_length()
{
	return maxvalue - minvalue;
}

char* BC_ISlider::get_caption()
{
	sprintf(caption, "%ld", value);
	return caption;
}

int BC_ISlider::increase_value()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	button_pixel = value_to_pixel();
	return 0;
}

int BC_ISlider::decrease_value()
{
	value--;
	if(value < minvalue) value = minvalue;
	button_pixel = value_to_pixel();
	return 0;
}

int BC_ISlider::init_selection(int cursor_x, int cursor_y)
{
	if(vertical)
	{
		min_pixel = -(int)((1.0 - (double)(value - minvalue) / (maxvalue - minvalue)) * pointer_motion_range);
		min_pixel += cursor_y;
	}
	else
	{
		min_pixel = -(int)((double)(value - minvalue) / (maxvalue - minvalue) * pointer_motion_range);
		min_pixel += cursor_x;
	}
	max_pixel = min_pixel + pointer_motion_range;
	return 0;
}

int BC_ISlider::update_selection(int cursor_x, int cursor_y)
{
	long old_value = value;

	if(vertical)
	{
		value = (long)((1.0 - (double)(cursor_y - min_pixel) / 
			pointer_motion_range) * 
			(maxvalue - minvalue) +
			minvalue);
	}
	else
	{
		value = (long)((double)(cursor_x - min_pixel) / 
			pointer_motion_range * 
			(maxvalue - minvalue) +
			minvalue);
	}

	if(value > maxvalue) value = maxvalue;
	if(value < minvalue) value = minvalue;
	button_pixel = value_to_pixel();

	if(old_value != value)
	{
		return 1;
	}
	return 0;
}

int BC_ISlider::handle_event()
{
	if(output) *output = get_value();
	return 1;
}








BC_FSlider::BC_FSlider(int x, 
			int y,
			int vertical,
			int pixels, 
			int pointer_motion_range, 
			float minvalue, 
			float maxvalue, 
			float value,
			int use_caption,
			VFrame **data)
 : BC_Slider(x, 
		y, 
		pixels, 
		pointer_motion_range,  
		data,
		1, 
		vertical,
		use_caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

int BC_FSlider::value_to_pixel()
{
//printf("BC_FSlider::value_to_pixel %f %f\n", maxvalue, minvalue);
	if(maxvalue == minvalue) return 0;
	{
		if(vertical)
			return (int)((1.0 - (double)(value - minvalue) / (maxvalue - minvalue)) * 
				(get_h() - get_button_pixels()));
		else
			return (int)((double)(value - minvalue) / (maxvalue - minvalue) * 
				(get_w() - get_button_pixels()));
	}
}

int BC_FSlider::update(float value)
{
	this->value = value;
	int old_pixel = button_pixel;
	button_pixel = value_to_pixel();
	if(button_pixel != old_pixel) draw_face();
	return 0;
}

int BC_FSlider::update(int pointer_motion_range, float value, float minvalue, float maxvalue)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
	this->pointer_motion_range = pointer_motion_range;
	int old_pixel = button_pixel;
	button_pixel = value_to_pixel();
	if(button_pixel != old_pixel) draw_face();
	return 0;
}


float BC_FSlider::get_value()
{
	return value;
}

float BC_FSlider::get_length()
{
	return maxvalue - minvalue;
}

char* BC_FSlider::get_caption()
{
	sprintf(caption, "%.02f", value);
	return caption;
}

int BC_FSlider::increase_value()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	button_pixel = value_to_pixel();
	return 0;
}

int BC_FSlider::decrease_value()
{
	value--;
	if(value < minvalue) value = minvalue;
	button_pixel = value_to_pixel();
	return 0;
}

int BC_FSlider::init_selection(int cursor_x, int cursor_y)
{
	if(vertical)
	{
		min_pixel = -(int)((1.0 - (double)(value - minvalue) / (maxvalue - minvalue)) * pointer_motion_range);
		min_pixel += cursor_y;
	}
	else
	{
		min_pixel = -(int)((double)(value - minvalue) / (maxvalue - minvalue) * pointer_motion_range);
		min_pixel += cursor_x;
	}
	max_pixel = min_pixel + pointer_motion_range;
	return 0;
}

int BC_FSlider::update_selection(int cursor_x, int cursor_y)
{
	float old_value = value;

	if(vertical)
	{
		value = (long)((1.0 - (double)(cursor_y - min_pixel) / 
			pointer_motion_range) * 
			(maxvalue - minvalue) +
			minvalue);
	}
	else
	{
		value = (long)((double)(cursor_x - min_pixel) / 
			pointer_motion_range * 
			(maxvalue - minvalue) +
			minvalue);
	}

	if(value > maxvalue) value = maxvalue;
	if(value < minvalue) value = minvalue;
	button_pixel = value_to_pixel();

	if(old_value != value)
	{
		return 1;
	}
	return 0;
}

BC_PercentageSlider::BC_PercentageSlider(int x, 
			int y,
			int vertical,
			int pixels, 
			int pointer_motion_range, 
			float minvalue, 
			float maxvalue, 
			float value,
			int use_caption,
			VFrame **data)
 : BC_FSlider(x, 
			y,
			vertical,
			pixels, 
			pointer_motion_range, 
			minvalue, 
			maxvalue, 
			value,
			use_caption,
			data)
{
}

char* BC_PercentageSlider::get_caption()
{
	sprintf(caption, "%.0f%%", (value - minvalue) / (maxvalue - minvalue) * 100);
	return caption;
}


