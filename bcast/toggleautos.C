#include <string.h>
#include "filehtal.h"
#include "toggleauto.h"
#include "toggleautos.h"

#define MINSTACKHEIGHT 16

ToggleAutos::ToggleAutos(Track *track, 
			int color, 
			int default_,
			int stack_number, 
			int stack_total)
 : Autos(track, color, default_, stack_number, stack_total)
{
// 1 is on            -1 is off
	this->max = 1; this->min = -1;
	this->virtual_h = 100;
}

ToggleAutos::~ToggleAutos()
{
}

int ToggleAutos::slope_adjustment(long ax, float slope)
{
	return 0;
return 0;
}

int ToggleAutos::get_track_pixels(int zoom_track, int pixel, int &center_pixel, float &yscale)
{
	if(zoom_track < MINSTACKHEIGHT)
	{
		center_pixel = pixel + zoom_track / 2;
		yscale = -(float)zoom_track / (max - min) * .75;
	}
	else
	if(zoom_track / stack_total < MINSTACKHEIGHT)
	{
		center_pixel = pixel + MINSTACKHEIGHT / 2 + (stack_number * MINSTACKHEIGHT % zoom_track) * zoom_track;
		yscale = -(float)MINSTACKHEIGHT / (max - min) * .75;
	}
	else
	{
		center_pixel = pixel + (zoom_track / stack_total) / 2 + (zoom_track / stack_total) * stack_number;
		yscale = -(float)(zoom_track / stack_total) / (max - min) * .75;
	}
return 0;
}

int ToggleAutos::draw_joining_line(BC_Canvas *canvas, int vertical, int center_pixel, int x1, int y1, int x2, int y2)
{
	if(vertical)
	canvas->draw_line(center_pixel - y1, x1, center_pixel - y1, x2);
	else
	canvas->draw_line(x1, center_pixel + y1, x2, center_pixel + y1);
	
	if(y1 != y2)
	{
		if(vertical)
		canvas->draw_line(center_pixel - y1, x2, center_pixel - y2, x2);
		else
		canvas->draw_line(x2, center_pixel + y1, x2, center_pixel + y2);
	}
return 0;
}


Auto* ToggleAutos::add_auto(long position, float value)
{
	ToggleAuto* current = (ToggleAuto*)autoof(position);
	ToggleAuto* new_auto;
	
	insert_before(current, new_auto = new ToggleAuto(this));

	new_auto->position = position;
	new_auto->value = value;
	
	return new_auto;
}


Auto* ToggleAutos::append_auto()
{
	return append(new ToggleAuto(this));
}


float ToggleAutos::fix_value(float value)
{
	if(value >= 0) value = 1;
	else
	if(value < 0) value = -1;
	return value;	
}

int ToggleAutos::get_testy(float slope, int cursor_x, int ax, int ay)
{
	return ay;
return 0;
}

int ToggleAutos::dump()
{
return 0;
}
