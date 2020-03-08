#include <math.h>

#include "bcpan.h"
#include "bcpixmap.h"
#include "bcresources.h"
#include "colors.h"


BC_Pan::BC_Pan(int x, 
		int y, 
		int virtual_r, 
		float maxvalue, 
		int total_values, 
		int *value_positions, 
		int stick_x, 
		int stick_y,
		float *values)
 : BC_SubWindow(x, y, -1, -1, -1)
{
	this->virtual_r = virtual_r;
	this->maxvalue = maxvalue;
	this->total_values = total_values;
	this->value_positions = new int[total_values];
	this->stick_x = stick_x;
	this->stick_y = stick_y;
	this->values = new float[total_values];
	highlighted = 0;
}

BC_Pan::~BC_Pan()
{
	delete values;
	delete value_positions;
	delete value_x;
	delete value_y;
	delete bg_pixmap;
	delete bg_pixmap_hi;
	delete handle_pixmap;
	delete channel_pixmap;
}

int BC_Pan::initialize()
{
	bg_pixmap = new BC_Pixmap(parent_window, get_resources()->pan_bg, PIXMAP_ALPHA);
	bg_pixmap_hi = new BC_Pixmap(parent_window, get_resources()->pan_bg_hi, PIXMAP_ALPHA);
	handle_pixmap = new BC_Pixmap(parent_window, get_resources()->pan_stick, PIXMAP_ALPHA);
	channel_pixmap = new BC_Pixmap(parent_window, get_resources()->pan_channel, PIXMAP_ALPHA);
	w = bg_pixmap->get_w();
	h = bg_pixmap->get_h();
	BC_SubWindow::initialize();
	draw();
	return 0;
}

int BC_Pan::update(int x, int y)
{
	stick_x = x;
	stick_y = y;
	stick_to_values();
	draw();
	return 0;
}

int BC_Pan::draw()
{
	if(highlighted)
	{
		draw_pixmap(bg_pixmap_hi);
	}
	else
	{
		draw_pixmap(bg_pixmap);
	}

// draw channels
	int x1, y1, x2, y2, w, h, j;
	float scale = (float)get_w() / virtual_r * 2;
	w = channel_pixmap->get_w();
	h = channel_pixmap->get_h();
	set_color(RED);
	for(int i = 0; i < total_values; i++)
	{
		x1 = (int)(value_x[i] * scale) - w / 2;
		y1 = (int)(value_y[i] * scale) - h / 2;
		x2 = x1 + w;
		y2 = y1 + h;
		if(x1 < 0) { x1 = 0; }
		if(y1 < 0) { y1 = 0; }
		if(x2 > get_w()) { x1 = get_w() - w; }
		if(y2 > get_w()) { y1 = get_w() - h; }
		if(y2 < h) { y2 = h; y1 = 0; }
		draw_pixmap(channel_pixmap, x1, y1);
	}

// draw stick
	set_color(MEYELLOW);
	w = h = 6;
	x1 = (int)(stick_x * scale);
	x2 = x1;
	y1 = (int)(stick_y * scale - h);
	y2 = y1 + h * 2;
	draw_line(x1, y1, x2, y2);

	x1 = (int)(stick_x * scale - w);
	x2 = x1 + w * 2;
	y1 = (int)(stick_y * scale);
	y2 = y1;
	draw_line(x1, y1, x2, y2);

	flash();
	return 0;
}

int BC_Pan::stick_to_values()
{
// find shortest distance to a channel
	float shortest = 2 * virtual_r, test_distance;
	int i;

	for(i = 0; i < total_values; i++)
	{
		if((test_distance = distance(stick_x, value_x[i], stick_y, value_y[i])) < shortest)
			shortest = test_distance;
	}

// get values for channels
	if(shortest == 0)
	{
		for(i = 0; i < total_values; i++)
		{
			if(distance(stick_x, value_x[i], stick_y, value_y[i]) == shortest)
				values[i] = maxvalue;
			else
				values[i] = 0;
		}
	}
	else
	{
		for(i = 0; i < total_values; i++)
		{
			values[i] = shortest;
			values[i] -= (float)(distance(stick_x, value_x[i], stick_y, value_y[i]) - shortest);
			if(values[i] < 0) values[i] = 0;
			values[i] = values[i] / shortest * maxvalue;
		}
	}
	return 0;
}

float BC_Pan::distance(int x1, int x2, int y1, int y2)
{
	return hypot(x2 - x1, y2 - y1);
}

int BC_Pan::change_channels(int new_channels, int *value_positions)
{
	delete values;
	delete this->value_positions;
	delete value_x;
	delete value_y;
	
	values = new float[new_channels];
	this->value_positions = new int[new_channels];
	value_x = new int[new_channels];
	value_y = new int[new_channels];
	total_values = new_channels;
	for(int i = 0; i < new_channels; i++)
	{
		this->value_positions[i] = value_positions[i];
	}
	get_channel_positions();
	stick_to_values();
	draw();
	return 0;
}

int BC_Pan::get_channel_positions()
{
	for(int i = 0; i < total_values; i++)
	{
		rdtoxy(value_x[i], value_y[i], virtual_r, value_positions[i], virtual_r);
	}
	return 0;
}

int BC_Pan::rdtoxy(int &x, int &y, int r, int a, int virtual_r)
{
	float radians = (float)(a - 90) / 360 * 2 * M_PI;
	y = (int)(sin(radians) * r);
	x = (int)(cos(radians) * r);
	x += virtual_r;
	y += virtual_r;
	return 0;
}

int BC_Pan::get_total_values()
{
	return total_values;
}

float BC_Pan::get_value(int channel)
{
	return values[channel];
}

int BC_Pan::get_stick_x() 
{ 
	return stick_x; 
}

int BC_Pan::get_stick_y() 
{ 
	return stick_y; 
}
