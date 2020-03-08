#ifndef AUTO_H
#define AUTO_H

#include "bcbase.h"
#include "filehtal.inc"
#include "autos.inc"

class Auto : public ListItem<Auto>
{
public:
	Auto() { };
	Auto(Autos *autos);
	virtual ~Auto() { };

	virtual int copy(long start, long end, FileHTAL *htal);

	virtual int save(FileHTAL *htal);
	virtual int load(FileHTAL *htal);

	virtual int draw(BC_Canvas *canvas, int x, int y, int center_pixel, int zoom_track, int vertical, int show_value);
	int selected(int ax, int ay, int cursor_x, int cursor_y, int center_pixel, int zoom_track);
	
	float value;
	long position;
	int skip;       // if added by selection event for moves
	Autos *autos;
	int WIDTH, HEIGHT;

private:
	virtual int value_to_str(char *string, float value) { return 0; };
};



#endif
