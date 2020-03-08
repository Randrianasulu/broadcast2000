#include <string.h>
#include "filehtal.h"
#include "floatauto.h"
#include "floatautos.h"

FloatAutos::FloatAutos(Track *track, 
				int color, 
				float min, 
				float max, 
				int virtual_h,
				int use_floats)
 : Autos(track, color, 0)
{
	this->max = max; this->min = min;
	this->virtual_h = virtual_h;
	this->use_floats = use_floats;
}

FloatAutos::~FloatAutos()
{
}

int FloatAutos::get_track_pixels(int zoom_track, int pixel, int &center_pixel, float &yscale)
{
	center_pixel = pixel + zoom_track / 2;
	yscale = -(float)zoom_track / (max - min);
return 0;
}

int FloatAutos::draw_joining_line(BC_Canvas *canvas, int vertical, int center_pixel, int x1, int y1, int x2, int y2)
{
	if(vertical)
	canvas->draw_line(center_pixel - y1, x1, center_pixel - y2, x2);
	else
	canvas->draw_line(x1, center_pixel + y1, x2, center_pixel + y2);
return 0;
}

Auto* FloatAutos::add_auto(long position, float value)
{
	FloatAuto* current = (FloatAuto*)autoof(position);
	FloatAuto* new_auto;
	
	insert_before(current, new_auto = new FloatAuto(this));

	new_auto->position = position;
	new_auto->value = value;
	
	return new_auto;
}

Auto* FloatAutos::append_auto()
{
	return append(new FloatAuto(this));
}

float FloatAutos::fix_value(float value)
{
	int value_int;
	
	if(use_floats)
	{
// Fix precision
		value_int = (int)(value * 100);
		value = (float)value_int / 100;
	}
	else
	{
// not really floating point
		value_int = (int)value;
		value = value_int;
	}

	if(value < min) value = min;
	else
	if(value > max) value = max;
	
	return value;	
}

int FloatAutos::get_testy(float slope, int cursor_x, int ax, int ay)
{
	return (int)(slope * (cursor_x - ax)) + ay;
return 0;
}

int FloatAutos::dump()
{
	printf("	FloatAutos::dump\n");
	for(Auto* current = first; current; current = NEXT)
	{
		printf("	position %ld\n", current->position);
	}
return 0;
}
