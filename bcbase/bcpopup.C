#include <string.h>
#include "bcpopup.h"

BC_Popup::BC_Popup(BC_Window *top_level, int color, int x, int y, int w, int h, int top_window_coords)
{
	XSetWindowAttributes attr;
	unsigned long mask;
	Cursor arrow;
	XSizeHints size_hints;
	Window tempwin;
	int x_, y_;

	this->top_level = top_level;
	this->w = w; this->h = h;

	mask = CWEventMask | 
		CWBackPixel | 
		CWBorderPixel | 
		CWColormap | 
		CWCursor | 
		CWOverrideRedirect | 
		CWSaveUnder;
	attr.event_mask = 
	 LeaveWindowMask
	 | ExposureMask
	 | ButtonPressMask
	 | ButtonReleaseMask
	 | StructureNotifyMask
	 | KeyPressMask
	 | PointerMotionMask;

	attr.background_pixel = top_level->get_color(color);
	attr.border_pixel = top_level->get_color(color);
	attr.colormap = top_level->cmap;
	arrow = XCreateFontCursor(top_level->display, XC_top_left_arrow);
	attr.cursor = arrow;
	attr.override_redirect = True;
	attr.save_under = True;

	if(top_window_coords)
		XTranslateCoordinates(top_level->display, top_level->win, top_level->rootwin, x, y, &x_, &y_, &tempwin);
	else
		{ x_ = x; y_ = y; }

	this->x = x; this->y = y;
// All coords are now relative to root.

	win = XCreateWindow(top_level->display, top_level->rootwin, x_, y_, w,  h, 0, top_level->depth, InputOutput, top_level->vis, mask, &attr);
	pixmap = XCreatePixmap(top_level->display, win, w,  h, top_level->depth);
	XRaiseWindow(top_level->display, win);
	XMapWindow(top_level->display, win);
	top_level->add_window(win);
}

BC_Popup::~BC_Popup()
{
// set this window as last deleted so last motion events from it don't get dispatched
	if(top_level->win)
	{
		top_level->set_last_deleted(win);
		XFreePixmap(top_level->display, pixmap);
		top_level->delete_window(win);
		XDestroyWindow(top_level->display, win);
	}
}

int BC_Popup::draw_text(int x_, int y_, const char *text) 
{
	XDrawString(top_level->display, pixmap, top_level->gc, x_, y_, text, strlen(text));
return 0;
}

int BC_Popup::get_text_width(XFontStruct *font, char *text)
{
	return XTextWidth(font, text, strlen(text));
return 0;
}

int BC_Popup::draw_line(int x1, int y1, int x2, int y2) 
{
	XDrawLine(top_level->display, pixmap, top_level->gc, x1, y1, x2, y2);
return 0;
}

int BC_Popup::set_color(int color)
{
	XSetForeground(top_level->display, top_level->gc, top_level->get_color(color)); 
return 0;
}

int BC_Popup::draw_box(int x_, int y_, int w_, int h_) 
{ 
	XFillRectangle(top_level->display, pixmap, top_level->gc, x_, y_, w_, h_); 
return 0;
}

int BC_Popup::draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow)
{
	int lx,ly,ux,uy;

	h--; w--;

	lx = x1+1;  ly = y1+1;
	ux = x1+w-1;  uy = y1+h-1;

	set_color(middle);
	draw_box(x1, y1, w, h);

	set_color(light);
	draw_line(x1, y1, x1+w, y1);
	draw_line(x1, y1, x1, y1+h);
	draw_line(lx, ly, ux, ly);
	draw_line(lx, ly, lx, uy);

	set_color(shadow);
	draw_line(x1+w, y1, x1+w, y1+h);
	draw_line(x1, y1+h, x1+w, y1+h);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
return 0;
}


int BC_Popup::draw_3d_big(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
	int lx,ly,ux,uy;

	h--; w--;

	lx = x+1;  ly = y+1;
	ux = x+w-1;  uy = y+h-1;

	set_color(middle);
	draw_box(x, y, w, h);

	set_color(light1);
	draw_line(x, y, ux, y);
	draw_line(x, y, x, uy);
	set_color(light2);
	draw_line(lx, ly, ux - 1, ly);
	draw_line(lx, ly, lx, uy - 1);

	set_color(shadow1);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
	set_color(shadow2);
	draw_line(x+w, y, x+w, y+h);
	draw_line(x, y+h, x+w, y+h);
return 0;
}

int BC_Popup::draw_check(int x, int y, int w, int h)
{
	draw_line(x + 3, y + h/2+0, x + 6, y + h/2+2);
	draw_line(x + 3, y + h/2+1, x + 6, y + h/2+3);
	draw_line(x + 6, y + h/2+2, x + w - 4, y + h/2-3);
	draw_line(x + 3, y + h/2+2, x + 6, y + h/2+4);
	draw_line(x + 6, y + h/2+2, x + w - 4, y + h/2-3);
	draw_line(x + 6, y + h/2+3, x + w - 4, y + h/2-2);
	draw_line(x + 6, y + h/2+4, x + w - 4, y + h/2-1);
return 0;
}

int BC_Popup::draw_rectangle(int x_, int y_, int w_, int h_) 
{ 
	XDrawRectangle(top_level->display, pixmap, top_level->gc, x_, y_, w_ - 1, h_ - 1); 
return 0;
}

int BC_Popup::flash()
{
  XCopyArea(top_level->display, pixmap, win, top_level->gc, 0, 0, w, h, 0, 0);
  XFlush(top_level->display);
return 0;
}

int BC_Popup::button_release_dispatch()
{
//printf("cursor_x %d cursor_y %d\n", top_level->cursor_x, top_level->cursor_y);
	cursor_x = top_level->cursor_x - x; cursor_y = top_level->cursor_y - y;
	button_release();
return 0;
}

int BC_Popup::button_press_dispatch()
{
	cursor_x = top_level->cursor_x - x; cursor_y = top_level->cursor_y - y;
	button_press();
return 0;
}

int BC_Popup::cursor_left_dispatch()
{
	cursor_left();
return 0;
}


int BC_Popup::expose_event_dispatch()
{
	if(top_level->event_win == win) flash();
return 0;
}

int BC_Popup::motion_event_dispatch()
{
	cursor_x = top_level->cursor_x - x; cursor_y = top_level->cursor_y - y;
//printf("cursor_x %d, top_level->cursor_x %d, cursor_y %d, top_level->cursor_y %d\n", cursor_x, top_level->cursor_x, cursor_y, top_level->cursor_y);
	cursor_motion();
return 0;
}

int BC_Popup::resize_window(int x, int y, int w, int h)
{
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
	this->x = x; this->y = y; this->w = w; this->h = h;
return 0;
}
