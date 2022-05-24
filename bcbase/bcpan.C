#include <string.h>
#include <math.h>

#include "bccolors.h"
#include "bcfont.h"
#include "bckeys.h"
#include "bcpan.h"
#include "bcresources.h"
#include "bcwindow.h"





BC_Pan::BC_Pan(int x, int y, int r, int virtual_r, float maxvalue, int total_values, int *value_positions, int stick_x, int stick_y)
 : BC_Tool(x, y, r * 2, r * 2)
{
	initialize(r, virtual_r, maxvalue, total_values, value_positions);
	this->stick_x = stick_x + virtual_r;
	this->stick_y = stick_y + virtual_r;
	get_values();
}

BC_Pan::BC_Pan(int x, int y, int r, int virtual_r, float maxvalue, int total_values, int *value_positions, float *values)
 : BC_Tool(x, y, r * 2, r * 2)
{
	initialize(r, virtual_r, maxvalue, total_values, value_positions);
	for(int i = 0; i < total_values; i++)
	{
		this->values[i] = values[i];
	}
	get_stick_position(total_values, value_positions, values, maxvalue, virtual_r);
}

int BC_Pan::initialize(int r, int virtual_r, float maxvalue, int total_values, int *value_positions)
{
	this->r = r;
	this->virtual_r = virtual_r;
	this->maxvalue = maxvalue;
	this->total_values = total_values;
	this->values = new float[total_values];
	this->value_positions = new int[total_values];
	value_x = new int[total_values];
	value_y = new int[total_values];
	highlighted = 0;
	button_down = 0;
	scale = (float)r / virtual_r;
	for(int i = 0; i < total_values; i++)
	{
		this->value_positions[i] = value_positions[i];
	}
	get_channel_positions();
return 0;
}

BC_Pan::~BC_Pan()
{
	delete values;
	delete value_positions;
	delete value_x;
	delete value_y;
}

int BC_Pan::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_Pan::draw()
{
	int ltface;
	int dkface;
	int background;

	if(highlighted)
	{
		draw_3d_big(0, 0, w, h, 
			top_level->get_resources()->button_shadow,
			RED,
			BLACK,
			PINK,
			top_level->get_resources()->button_light);
	  //ltface = LTGREY;
	  //dkface = MDGREY;
	  //background = DKGREY;
	}
	else
	{
		draw_3d_big(0, 0, w, h, 
			top_level->get_resources()->button_shadow,
			BLACK,
			BLACK,
			top_level->get_resources()->button_shadow,
			top_level->get_resources()->button_light);
	  //background = BLACK;
	}
	background = BLACK;

	//draw_3d_big(0, 0, w, h, dkface, background, ltface);

// draw channels
	int x1, y1, x2, y2, w, h, j;
	w = 16;
	h = 9;
	set_color(RED);
	for(int i = 0; i < total_values; i++)
	{
		x1 = (int)(value_x[i] * scale) - w / 2;
		y1 = (int)(value_y[i] * scale) - h / 2;
		x2 = x1 + w;
		y2 = y1 + h;
		if(x1 < 0)     { x1 = 0; }
		if(y1 < 0)     { y1 = 0; }
		if(x2 > 2 * r) { x1 = 2 * r - w; }
		if(y2 > 2 * r) { y1 = 2 * r - h; }
		if(y2 < h) { y2 = h; y1 = 0; }

		sprintf(string, "%f", values[i]);
		if(values[i] < 1) 
		{
			j = 1;
			string[4] = 0;
		}
		else 
		{
			j = 0;
			string[1] = 0;
		}
		set_font(SMALLFONT);
		draw_text(x1, y2, &string[j]);
		set_font(MEDIUMFONT);
		//draw_box(x1, y1, w, w);
	}

// draw stick
//printf("BC_Pan::draw x %d y %d\n", stick_x, stick_y);
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

int BC_Pan::cursor_left_()
{
	if(highlighted)
	{
		if(cursor_x < 0 || cursor_x > w ||
			 cursor_y < 0 || cursor_y > h)
		{   // draw unhighlighted
			highlighted = 0;
			draw();
		}
	}
	return 0;
return 0;
}

int BC_Pan::cursor_motion_()
{
	int result;
	result = 0;

	if(button_down)
	{
		stick_x = cursor_x - x_adjust;
		stick_y = cursor_y - y_adjust;
		if(stick_x < 0) stick_x = 0;
		if(stick_x > virtual_r * 2) stick_x = virtual_r * 2;
		if(stick_y < 0) stick_y = 0;
		if(stick_y > virtual_r * 2) stick_y = virtual_r * 2;
		
		get_values();
		handle_event();
		draw();
		result = 1;
	}
	else
	{
  		if(get_event_win() != get_top_win()) return 0;
  	
		if(cursor_x > 0 && cursor_x < w &&
			 cursor_y > 0 && cursor_y < h)
		{   // draw highlighted
			result = 1;
			if(!highlighted)
			{
				top_level->unhighlight();
				highlighted = 1;
				draw();
			}
		}
		else
		if(highlighted)
		{	// draw unhighlighted
			unhighlight();
		}
	}
	return result;
return 0;
}


int BC_Pan::button_press_()
{
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		x_adjust = cursor_x - stick_x;
		y_adjust = cursor_y - stick_y;
		button_down = 1;
		if(get_active_tool() != this)
		{
			activate();
		}
		return 1;
	}
	return 0;
return 0;
}

int BC_Pan::button_release_()
{
	button_down = 0;
return 0;
}

int BC_Pan::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		draw();
	}
	return 0;
return 0;
}

int BC_Pan::update(int x, int y)
{
	stick_x = x;
	stick_y = y;
	get_values();
	draw();
return 0;
}

int BC_Pan::resize_tool(int x, int y)
{
	resize_window(x, y, w, h);
	draw();
return 0;
}

int BC_Pan::keypress_event_()
{
	int result;
	result = 0;

//printf("BC_Pan::keypress_event_ 1\n");
	if(get_active_tool() == this)
	{
//printf("BC_Pan::keypress_event_ 2\n");
		switch(top_level->get_keypress())
		{
			case LEFT:       stick_x--; result = 1;             break;
			case RIGHT:      stick_x++; result = 1;             break;
			case DOWN:       stick_y++; result = 1;             break;
			case UP:         stick_y--; result = 1;             break;
		}
	}
	if(result) { trap_keypress(); get_values(); handle_event(); draw(); }
	return result;
return 0;
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
	get_values();
	draw();
return 0;
}


int BC_Pan::get_values()
{
// find shortest distance to a channel
	int shortest = 2 * virtual_r, test_distance;
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

int BC_Pan::get_stick_position(int total_values, int *value_positions, float *values, float maxvalue, int virtual_r)
{
// get highest value
	int highest_number;
	float highest_value = 0, next_highest = 0;
	int angle = 0;
	int i, j;

	for(i = 0; i < total_values; i++)
	{
		if(values[i] > highest_value)
		{
			highest_value = values[i];
			angle = value_positions[i];
			highest_number = i;
		}
	}
	
// get next highest
	for(j = 0; j < total_values; j++)
	{
		if(values[i] < highest_value && values[i] > next_highest)
		{
			next_highest = values[i];
		}
	}

	float radius = 1 - next_highest / maxvalue;
	radius *= virtual_r;
	rdtoxy(stick_x, stick_y, (int)radius, angle, virtual_r);
//printf("BC_Pan::get_stick_position x %d y %d\n", stick_x, stick_y);
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
	
//printf("a %d r %d x %d y %d virtual_r %d sin(radians) * r %d\n", a, r, x, y, virtual_r, (int)(sin(radians) * r));
return 0;
}

int BC_Pan::distance(int x1, int x2, int y1, int y2)
{
	return (int)hypot(x2 - x1, y2 - y1);
return 0;
}

int BC_Pan::get_stick_x() { return stick_x; return 0;
}
int BC_Pan::get_stick_y() { return stick_y; return 0;
}
float BC_Pan::get_value(int channel) { return values[channel]; }
int BC_Pan::get_total_values() { return total_values; return 0;
}
