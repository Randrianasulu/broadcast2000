#include <string.h>
#include "cursor.h"

Cursor_::Cursor_(BC_Canvas *canvas)
{
	this->canvas = canvas;
	selectionstart = selectionend = zoom_sample = viewstart = 0;
}

Cursor_::~Cursor_()
{
}

int Cursor_::show(int flash, long selectionstart, long selectionend, long zoom_sample, long viewstart, int vertical)
{
	this->selectionstart = selectionstart;
	this->selectionend = selectionend;
	this->viewstart = viewstart;
	this->zoom_sample = zoom_sample;
	this->vertical = vertical;
	draw(flash, selectionstart, selectionend, zoom_sample, viewstart, vertical);
return 0;
}

int Cursor_::hide(int flash)
{
	draw(flash, selectionstart, selectionend, zoom_sample, viewstart, vertical);
return 0;
}

int Cursor_::draw(int flash, long selectionstart, long selectionend, long zoom_sample, long viewstart, int vertical)
{

	if(canvas->h * canvas->w == 0) return 1; 
	if(zoom_sample == 0) return 1;       // no canvas

	long start = viewstart;
	long end = start + (long)(vertical ? canvas->h : canvas->w) * zoom_sample;
	int pixel1, pixel2;

	if((selectionstart >= start && selectionstart <= end) ||
		 (selectionend >= start && selectionend <= end) ||
		 (start >= selectionstart && end <= selectionend))
	{
		if(selectionstart < start)
		{
			pixel1 = 0;
		}
		else
		{
			pixel1 = (selectionstart - start) / zoom_sample;
		}
		
		if(selectionend > end)
		{
			pixel2 = (vertical ? canvas->h : canvas->w);
		}
		else
		{
			pixel2 = (selectionend - start) / zoom_sample;
		}
		pixel2++;

		canvas->set_inverse();
		canvas->set_color(WHITE);
		
		if(vertical)
		canvas->draw_box(0, pixel1, canvas->w, pixel2 - pixel1);
		else
		canvas->draw_box(pixel1, 0, pixel2 - pixel1, canvas->h);
		
		
		canvas->set_opaque();
	}
	if(flash) canvas->flash();
return 0;
}

int Cursor_::resize(int w, int h)
{
return 0;
}
