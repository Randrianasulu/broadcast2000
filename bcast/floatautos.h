#ifndef FLOATAUTOS_H
#define FLOATAUTOS_H

#include "autos.h"
#include "bcbase.h"
#include "filehtal.inc"

class FloatAutos : public Autos
{
public:
	FloatAutos(Track *track, 
				int color, 
				float min, 
				float max, 
				int virtual_h = AUTOS_VIRTUAL_HEIGHT,
				int use_floats = 0);
	~FloatAutos();

	int get_track_pixels(int zoom_track, int pixel, int &center_pixel, float &yscale);
	int draw_joining_line(BC_Canvas *canvas, int vertical, int center_pixel, int x1, int y1, int x2, int y2);
	float fix_value(float value);
	int get_testy(float slope, int cursor_x, int ax, int ay);

	int dump();
	Auto* add_auto(long position, float value);
	Auto* append_auto();
	int use_floats;
};


#endif
