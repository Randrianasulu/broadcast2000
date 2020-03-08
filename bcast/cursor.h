#ifndef CURSOR_H
#define CURSOR_H

#include "bcbase.h"

class Cursor_
{
public:
	Cursor_(BC_Canvas *canvas);
	~Cursor_();

	int resize(int w, int h);
	int show(int flash, long selectionstart, long selectionend, long zoom_sample, long viewstart, int vertical);
	int hide(int flash);
	int draw(int flash, long selectionstart, long selectionend, long zoom_sample, long viewstart, int vertical);

	BC_Canvas *canvas;
	int vertical;
	long selectionstart, selectionend, zoom_sample, viewstart;
};

#endif
