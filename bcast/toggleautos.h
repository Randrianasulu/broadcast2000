#ifndef TOGGLEAUTOS_H
#define TOGGLEAUTOS_H

#include "autos.h"
#include "bcbase.h"
#include "filehtal.inc"

class ToggleAutos : public Autos
{
public:
	ToggleAutos(Track *track, 
				int color, 
				int default_, 
				int stack_number = 0, 
				int stack_total = 1);
	~ToggleAutos();

	int slope_adjustment(long ax, float slope);
	int get_track_pixels(int zoom_track, int pixel, int &center_pixel, float &yscale);
	int draw_joining_line(BC_Canvas *canvas, int vertical, int center_pixel, int x1, int y1, int x2, int y2);
	float fix_value(float value);
	int get_testy(float slope, int cursor_x, int ax, int ay);
	int dump();

	Auto* add_auto(long position, float value);
	Auto* append_auto();
};


#endif
