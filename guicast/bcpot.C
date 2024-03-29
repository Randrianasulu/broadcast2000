#include "bcpot.h"
#include "bcresources.h"
#include "colors.h"
#include "keys.h"
#include "units.h"
#include "vframe.h"

#include <ctype.h>
#include <math.h>
#include <string.h>
#define MIN_ANGLE 225
#define MAX_ANGLE -45

BC_Pot::BC_Pot(int x, int y, VFrame **data)
 : BC_SubWindow(x, y, -1, -1, -1)
{
	this->data = data;
	for(int i = 0; i < POT_STATES; i++)
		images[i] = 0;
}

BC_Pot::~BC_Pot()
{
}

int BC_Pot::initialize()
{
	if(!data)
	{
		data = get_resources()->pot_images;
	}

	status = POT_UP;
	set_data(data);
	w = data[0]->get_w();
	h = data[0]->get_h();
	BC_SubWindow::initialize();
	draw();
	return 0;
}

int BC_Pot::reposition_window(int x, int y)
{
	BC_WindowBase::reposition_window(x, y);
	draw();
	return 0;
}

int BC_Pot::set_data(VFrame **data)
{
	for(int i = 0; i < POT_STATES; i++)
		if(images[i]) delete images[i];

	for(int i = 0; i < POT_STATES; i++)
		images[i] = new BC_Pixmap(parent_window, data[i], PIXMAP_ALPHA);
	return 0;
}


int BC_Pot::draw()
{
	int x1, y1, x2, y2;
	draw_top_background(parent_window, 0, 0, get_w(), get_h());
	draw_pixmap(images[status]);
	set_color(BLACK);

	angle_to_coords(x1, y1, x2, y2, percentage_to_angle(get_percentage()));
	draw_line(x1, y1, x2, y2);

	flash();
	return 0;
}

float BC_Pot::percentage_to_angle(float percentage)
{
	return percentage * (MAX_ANGLE - MIN_ANGLE) + MIN_ANGLE;
}

float BC_Pot::angle_to_percentage(float angle)
{
	return (angle - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE);
}


int BC_Pot::angle_to_coords(int &x1, int &y1, int &x2, int &y2, float angle)
{
	x1 = get_resources()->pot_x1;
	y1 = get_resources()->pot_y1;
	if(status == POT_DN)
	{
		x1 += 2;
		y1 += 2;
	}

	while(angle < 0) angle += 360;

	x2 = (int)(cos(angle / 360 * (2 * M_PI)) * get_resources()->pot_r + x1);
	y2 = (int)(-sin(angle / 360 * (2 * M_PI)) * get_resources()->pot_r + y1);
	return 0;
}

float BC_Pot::coords_to_angle(int x2, int y2)
{
	int x1, y1, x, y;
	float angle = 0;

	x1 = get_resources()->pot_x1;
	y1 = get_resources()->pot_y1;
	if(status == POT_DN)
	{
		x1 += 2;
		y1 += 2;
	}

	x = x2 - x1;
	y = y2 - y1;

	if(x > 0 && y <= 0)
	{
		angle = atan((float)-y / x) / (2 * M_PI) * 360;
	}
	else
	if(x < 0 && y <= 0)
	{
		angle = 180 - atan((float)-y / -x) / (2 * M_PI) * 360;
	}
	else
	if(x < 0 && y > 0)
	{
		angle = 180 - atan((float)-y / -x) / (2 * M_PI) * 360;
	}
	else
	if(x > 0 && y > 0)
	{
		angle = 360 + atan((float)-y / x) / (2 * M_PI) * 360;
	}
	else
	if(x == 0 && y < 0)
	{
		angle = 90;
	}
	else
	if(x == 0 && y > 0)
	{
		angle = 270;
	}
	else
	if(x == 0 && y == 0)
	{
		angle = 0;
	}

	return angle;
}




void BC_Pot::show_value_tooltip()
{
	set_tooltip(get_caption());
	show_tooltip(50);
	keypress_tooltip_timer = 2000;
}

int BC_Pot::repeat_event(long duration)
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
			if(status != POT_HIGH && status != POT_DN)
			{
				hide_tooltip();
			}
		}
		else
		if(status == POT_HIGH)
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

int BC_Pot::keypress_event()
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
		draw();
		handle_event();
	}
	return result;
}

int BC_Pot::cursor_enter_event()
{
	if(top_level->event_win == win)
	{
// Set caption if no tooltip
		tooltip_done = 0;
		if(!top_level->button_down && status == POT_UP)
		{
			status = POT_HIGH;
		}
		draw();
	}
	return 0;
}

int BC_Pot::cursor_leave_event()
{
	if(status == POT_HIGH)
	{
		status = POT_UP;
		draw();
		hide_tooltip();
	}
	return 0;
}

int BC_Pot::button_press_event()
{
	if(!tooltip_on) top_level->hide_tooltip();
	if(top_level->event_win == win)
	{
		if(status == POT_HIGH || status == POT_UP)
		{
			status = POT_DN;
			start_cursor_angle = coords_to_angle(get_cursor_x(), get_cursor_y());
			start_needle_angle = percentage_to_angle(get_percentage());
			angle_offset = start_cursor_angle - start_needle_angle;
			prev_angle = start_cursor_angle;
			angle_correction = 0;
			draw();
			top_level->deactivate();
			top_level->active_subwindow = this;
			show_value_tooltip();
			return 1;
		}
	}
	return 0;
}

int BC_Pot::button_release_event()
{
	if(top_level->event_win == win)
	{
		if(status == POT_DN)
		{
			if(cursor_inside())
				status = POT_HIGH;
			else
			{
				status = POT_UP;
				top_level->hide_tooltip();
			}
		}
		draw();
	}
	return 0;
}

int BC_Pot::cursor_motion_event()
{
	if(top_level->button_down && 
		top_level->event_win == win && 
		status == POT_DN)
	{
		float angle = coords_to_angle(get_cursor_x(), get_cursor_y());

		if(prev_angle >= 0 && prev_angle < 90 &&
			angle >= 270 && angle < 360)
		{
			angle_correction -= 360;
		}
		else
		if(prev_angle >= 270 && prev_angle < 360 &&
			angle >= 0 && angle < 90)
		{
			angle_correction += 360;
		}
		
		prev_angle = angle;

		if(percentage_to_value(angle_to_percentage(angle + angle_correction - angle_offset)))
		{
			set_tooltip(get_caption());
			draw();
			handle_event();
		}
		return 1;
	}
	return 0;
}











BC_FPot::BC_FPot(int x, 
	int y, 
	float value, 
	float minvalue, 
	float maxvalue, 
	VFrame **data)
 : BC_Pot(x, y, data)
{
	this->value = value;
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
}

BC_FPot::~BC_FPot()
{
}

int BC_FPot::increase_value()
{
	value += 0.1;
	if(value > maxvalue) value = maxvalue;
	return 0;
}

int BC_FPot::decrease_value()
{
	value -= 0.1;
	if(value < minvalue) value = minvalue;
	return 0;
}

char*  BC_FPot::get_caption()
{
	sprintf(caption, "%.1f", value);
	return caption;
}

float BC_FPot::get_percentage()
{
	return (value - minvalue) / (maxvalue - minvalue);
}

int BC_FPot::percentage_to_value(float percentage)
{
	float old_value = value;
	value = percentage * (maxvalue - minvalue) + minvalue;
	value = Units::quantize10(value);
	if(value < minvalue) value = minvalue;
	if(value > maxvalue) value = maxvalue;
	if(value != old_value) return 1;
	return 0;
}

float BC_FPot::get_value()
{
	return value;
}

void BC_FPot::update(float value)
{
	this->value = value;
	draw();
}








BC_IPot::BC_IPot(int x, 
	int y, 
	long value, 
	long minvalue, 
	long maxvalue, 
	VFrame **data)
 : BC_Pot(x, y, data)
{
	this->value = value;
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
}

BC_IPot::~BC_IPot()
{
}

int BC_IPot::increase_value()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	return 0;
}

int BC_IPot::decrease_value()
{
	value--;
	if(value < minvalue) value = minvalue;
	return 0;
}

char*  BC_IPot::get_caption()
{
	sprintf(caption, "%ld", value);
	return caption;
}

float BC_IPot::get_percentage()
{
	return ((float)value - minvalue) / (maxvalue - minvalue);
}

int BC_IPot::percentage_to_value(float percentage)
{
	long old_value = value;
	value = (long)(percentage * (maxvalue - minvalue) + minvalue);
	if(value < minvalue) value = minvalue;
	if(value > maxvalue) value = maxvalue;
	if(value != old_value) return 1;
	return 0;
}

long BC_IPot::get_value()
{
	return value;
}

void BC_IPot::update(long value)
{
	this->value = value;
	draw();
}








BC_PercentagePot::BC_PercentagePot(int x, 
	int y, 
	float value, 
	float minvalue, 
	float maxvalue, 
	VFrame **data)
 : BC_Pot(x, y, data)
{
	this->value = value;
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
}

BC_PercentagePot::~BC_PercentagePot()
{
}

int BC_PercentagePot::increase_value()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	return 0;
}

int BC_PercentagePot::decrease_value()
{
	value--;
	if(value < minvalue) value = minvalue;
	return 0;
}

char*  BC_PercentagePot::get_caption()
{
	sprintf(caption, "%d%%", (int)(get_percentage() * 100 + 0.5));
	return caption;
}

float BC_PercentagePot::get_percentage()
{
	return (value - minvalue) / (maxvalue - minvalue);
}

int BC_PercentagePot::percentage_to_value(float percentage)
{
	float old_value = value;
	value = percentage * (maxvalue - minvalue) + minvalue;
	if(value < minvalue) value = minvalue;
	if(value > maxvalue) value = maxvalue;
	if(value != old_value) return 1;
	return 0;
}

float BC_PercentagePot::get_value()
{
	return value;
}

void BC_PercentagePot::update(float value)
{
	this->value = value;
	draw();
}








