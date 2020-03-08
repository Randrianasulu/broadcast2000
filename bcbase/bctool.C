#include <string.h>
#include "bcbitmap.h"
#include "bccolors.h"
#include "bcfont.h"
#include "bcresources.h"
#include "bcsubwindow.h"
#include "bctool.h"
#include "bcwindow.h"
#include "colormodels.h"

BC_Tool::BC_Tool(int x_, int y_, int w_, int h_)
{
	x = x_; y = y_; w = w_; h = h_;
	pixmap = 0;
	win = 0;
	enabled = 1;
	list_item = 0;      // in case this tool is never incorporated into a window
	font_pixmap = 0;
	font_pixmap_w = font_pixmap_h = 0;
	font_bitmap = 0;
	image_bitmap = 0;
	repeat_id = -1;
}

BC_Tool::~BC_Tool()
{
// deactivate if active
	if(top_level->active_tool == this) top_level->active_tool = 0;

// destroy the X window
	if(font_pixmap)
	{
		XFreePixmap(top_level->display, font_pixmap);
		font_pixmap = 0;
	}

	if(font_bitmap)
	{
		delete font_bitmap;
		font_bitmap = 0;
	}

	if(image_bitmap)
	{
		delete image_bitmap;
		image_bitmap = 0;
	}

	if(top_level->win)
	{
		top_level->delete_window(win);
		XDestroyWindow(top_level->display, win);
		XFreePixmap(top_level->display, pixmap);

		XFlush(top_level->display);
	}

	if(list_item)
	{
		list_item->pointer = 0;   // stop an infinite loop
		delete list_item;        // delete the list item that owns this tool
	}
}

int BC_Tool::create_tool_objects(BC_Window *top_level, BC_WindowBase *subwindow)
{
	this->top_level = top_level;
	this->subwindow = subwindow;
	create_tool_objects();   // tool creates its window here
return 0;
}

int BC_Tool::create_tool_objects()
{
// copy the subwindow's color by default
	create_window(x, y, w, h, subwindow->get_color());
	create_objects();       // user can create objects here
return 0;
}

int BC_Tool::create_window(int x, int y, int w, int h, int color)
{
	this->color = color;
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;

// create the window
	unsigned long mask;
	XSetWindowAttributes attr;

	mask = CWEventMask | CWBackPixel | CWBorderPixel;
	attr.event_mask = ExposureMask;
	attr.background_pixel = top_level->get_color(color);
	attr.border_pixel = top_level->get_color(color);

	win = XCreateWindow(top_level->display, subwindow->win, x, y, w, h, 0, top_level->depth, InputOutput, top_level->vis, mask, &attr);

	XMapWindow(top_level->display, win);
	pixmap = XCreatePixmap(top_level->display, win, w, h, top_level->depth);
	top_level->add_window(win);
	set_color(color);
	draw_box(0, 0, w, h);
return 0;
}

int BC_Tool::button_release_dispatch()
{
	cursor_x = subwindow->cursor_x - x;
	cursor_y = subwindow->cursor_y - y;
	if(enabled) button_release_();
return 0;
}

int BC_Tool::button_press_dispatch()
{
	int result = 0;
	cursor_x = subwindow->cursor_x - x;
	cursor_y = subwindow->cursor_y - y;
	if(enabled) result = button_press_();
	return result;
return 0;
}

int BC_Tool::expose_event_dispatch()
{
// if window flashes black and white, gc is inverse
	if(top_level->event_win == win) flash();
return 0;
}

int BC_Tool::cursor_left_dispatch()
{
	cursor_x = subwindow->cursor_x - x;
	cursor_y = subwindow->cursor_y - y;
	if(enabled) cursor_left_();
return 0;
}

int BC_Tool::motion_event_dispatch()
{
	int result;
	result = 0;
// Just in case this tool is the active tool and called before subwindow
// adjusts its cursor paramaters, calculate our own cursor position.
	cursor_x = top_level->cursor_x - subwindow->get_x() - x;
	cursor_y = top_level->cursor_y - subwindow->get_y() - y;
	if(enabled) result = cursor_motion_();          // send to tool
	return result;
return 0;
}

int BC_Tool::repeat_event_dispatch(long repeat_id)
{
	if(repeat_id == this->repeat_id)
	{
		if(!repeat_()) repeat();
		return 1;
	}
	return 0;
return 0;
}

int BC_Tool::resize_window(int x, int y, int w, int h)
{
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
	XFreePixmap(top_level->display, pixmap);
	pixmap = XCreatePixmap(top_level->display, win, w, h, top_level->depth);
// Copy the dimensions.
	this->x = x; this->y = y; this->w = w; this->h = h;
// clear the pixmap
	set_color(color);
	draw_box(0, 0, w, h);
return 0;
}

int BC_Tool::enable() { enabled = 1; return 0;
}

int BC_Tool::disable() { enabled = 0; return 0;
}

int BC_Tool::disable_window() { top_level->disable_window(); return 0;
}

int BC_Tool::enable_window() { top_level->enable_window(); return 0;
}

int BC_Tool::lock_window() { top_level->lock_window(); return 0;
}

int BC_Tool::unlock_window() { top_level->unlock_window(); return 0;
}

int BC_Tool::change_y(int y)
{
	this->y += y;
	XMoveResizeWindow(top_level->display, win, x, this->y, w, h);
	change_y_(y);
return 0;
}

int BC_Tool::set_y(int y)
{
	this->y = y;
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
return 0;
}

int BC_Tool::set_x(int x)
{
	this->x = x;
	if(x < -w || x > subwindow->get_w())     // move just outside window if out of range
	XMoveResizeWindow(top_level->display, win, -w, y, w, h);
	else
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
return 0;
}

int BC_Tool::get_keypress()
{
	return top_level->key_pressed;
return 0;
}

int BC_Tool::get_buttonpress()
{
	return top_level->button_pressed;
return 0;
}

int BC_Tool::get_double_click()
{
	return top_level->get_double_click();
return 0;
}

int BC_Tool::get_cursor_x()
{
	return cursor_x;
return 0;
}

int BC_Tool::get_cursor_y()
{
	return cursor_y;
return 0;
}

int BC_Tool::ctrl_down()
{
	return top_level->ctrl_down();
return 0;
}

int BC_Tool::shift_down()
{
	return top_level->shift_down();
return 0;
}

int BC_Tool::trap_keypress()
{ top_level->key_pressed = 0; return 0;
}

int BC_Tool::set_done(int return_value)
{
	top_level->set_done(return_value);
return 0;
}

int BC_Tool::flash()
{
	set_opaque();
  XCopyArea(top_level->display, pixmap, win, top_level->gc, 0, 0, w, h, 0, 0);
  XFlush(top_level->display);
return 0;
};

int BC_Tool::flash(int x_, int y_, int w_, int h_)
{
	if(y_ < 0)
	{
		h_ += y_; y_ = 0;  
	}
	if(x_ < 0)
	{
		w_ += x_; x_ = 0;  
	}
	if(y_ + h_ > h) h_ = h - y_;
	if(x_ + w_ > w) w_ = w - x_;

  XCopyArea(top_level->display, pixmap, win, top_level->gc, x_, y_, w_, h_, x_, y_);
	XFlush(top_level->display);
return 0;
}

int BC_Tool::activate()
{
	if(!is_active())
	{
		if(top_level->active_tool) top_level->active_tool->deactivate();
		top_level->active_tool = this;
		activate_();
	}
return 0;
}

int BC_Tool::deactivate()
{
	if(is_active())
	{
		top_level->active_tool = 0;
		deactivate_();
		return 1;
	}
	return 0;
return 0;
}

int BC_Tool::unhighlight()
{
	unhighlight_();
	return 0;
return 0;
}


int BC_Tool::is_active()
{
	if(top_level->active_tool == this) return 1; else return 0;
return 0;
}





// ================================ repeating functions

int BC_Tool::set_repeat(long repeat)
{
	if(repeat_id == -1)
	{
// This tool is only allowed 1 repeater.
		repeat_id = top_level->new_repeat_id();
	}
	top_level->set_repeat(repeat, repeat_id);
return 0;
}

int BC_Tool::unset_repeat()
{
	top_level->unset_repeat(repeat_id);
// Make repeater available again.
	repeat_id = -1;
return 0;
}

long BC_Tool::get_repeat()
{
	return top_level->get_repeat(repeat_id);
}







int BC_Tool::set_color(int color, GC *gc)
{
	XSetForeground(top_level->display, gc ? *gc : top_level->gc, top_level->get_color(color)); 
return 0;
}

int BC_Tool::set_inverse() 
{
	XSetFunction(top_level->display, top_level->gc, GXxor);
return 0;
}

int BC_Tool::set_opaque() 
{
	XSetFunction(top_level->display, top_level->gc, GXcopy);
return 0;
}

int BC_Tool::draw_3d_diamond(int x1, int y1, int w, int h, int light, int middle, int shadow)
{
	int x2, x3, y2, y3;
	XPoint point[4];
	
	w--; h--;
	x2 = x1 + w / 2;
	x3 = x1 + w;
	y2 = y1 + h / 2;
	y3 = y1 + h;

	point[0].x = x1; point[0].y = y2;
	point[1].x = x2; point[1].y = y1;
	point[2].x = x3; point[2].y = y2;
	point[3].x = x2; point[3].y = y3;
  set_color(middle);
  XFillPolygon(top_level->display, pixmap, top_level->gc, point, 4, Nonconvex, CoordModeOrigin);
	
  set_color(light);
	draw_line(x1, y2, x2, y1);
	draw_line(x1+1, y2, x2+1, y1);
	
	draw_line(x2, y1, x3, y2);
	draw_line(x2+1, y1, x3+1, y2);
	
  set_color(shadow);
	draw_line(x3, y2, x2, y3);
	draw_line(x3+1, y2, x2+1, y3);
	
	draw_line(x2, y3, x1, y2);
	draw_line(x2+1, y3, x1+1, y2);
return 0;
}

int BC_Tool::draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow)
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

int BC_Tool::draw_3d_big(int x, int y, int w, int h, 
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

int BC_Tool::draw_3d_border(int x, int y, int w, int h, 
	int light1, int light2, int shadow1, int shadow2)
{
	int lx,ly,ux,uy;

	h--; w--;

	lx = x+1;  ly = y+1;
	ux = x+w-1;  uy = y+h-1;

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

int BC_Tool::draw_triangle_up(int x, int y, int w, int h, 
	int light, int middle, int shadow)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; y1 = y; x2 = x + w / 2;
	y2 = y + h - 1; x3 = x + w - 1;

// middle
	point[0].x = x2; point[0].y = y1; point[1].x = x3;
	point[1].y = y2; point[2].x = x1; point[2].y = y2;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// bottom and top right
	set_color(shadow);
	draw_line(x3, y2, x1, y2);
	draw_line(x2, y1, x3, y2);

// top left
	set_color(light);
	draw_line(x2, y1, x1, y2);
return 0;
}

int BC_Tool::draw_triangle_down(int x, int y, int w, int h, 
	int light, int middle, int shadow)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; x2 = x + w / 2; x3 = x + w - 1;
	y1 = y; y2 = y + h - 1;

	point[0].x = x2; point[0].y = y2; point[1].x = x3;
	point[1].y = y1; point[2].x = x1; point[2].y = y1;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// top and bottom left
	set_color(light);
	draw_line(x3, y1, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow);
	draw_line(x3, y1, x2, y2);
return 0;
}

int BC_Tool::draw_triangle_up(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; y1 = y; x2 = x + w / 2;
	y2 = y + h - 1; x3 = x + w - 1;

// middle
	point[0].x = x2; point[0].y = y1; point[1].x = x3;
	point[1].y = y2; point[2].x = x1; point[2].y = y2;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// bottom and top right
	set_color(shadow1);
	draw_line(x3, y2-1, x1, y2-1);
	draw_line(x2-1, y1, x3-1, y2);
	set_color(shadow2);
	draw_line(x3, y2, x1, y2);
	draw_line(x2, y1, x3, y2);

// top left
	set_color(light2);
	draw_line(x2+1, y1, x1+1, y2);
	set_color(light1);
	draw_line(x2, y1, x1, y2);
return 0;
}

int BC_Tool::draw_triangle_down(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; x2 = x + w / 2; x3 = x + w - 1;
	y1 = y; y2 = y + h - 1;

	point[0].x = x2; point[0].y = y2; point[1].x = x3;
	point[1].y = y1; point[2].x = x1; point[2].y = y1;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// top and bottom left
	set_color(light2);
	draw_line(x3-1, y1+1, x1+1, y1+1);
	draw_line(x1+1, y1, x2+1, y2);
	set_color(light1);
	draw_line(x3, y1, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow1);
  	draw_line(x3-1, y1, x2-1, y2);
	set_color(shadow2);
	draw_line(x3, y1, x2, y2);
return 0;
}

int BC_Tool::draw_triangle_left(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
  	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	// draw back arrow
  	y1 = y; x1 = x; y2 = y + h / 2;
  	x2 = x + w - 1; y3 = y + h - 1;

	point[0].x = x1; point[0].y = y2; point[1].x = x2; 
	point[1].y = y1; point[2].x = x2; point[2].y = y3;

	set_color(middle);
  	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// right and bottom right
	set_color(shadow1);
  	draw_line(x2-1, y1, x2-1, y3-1);
  	draw_line(x2, y3-1, x1, y2-1);
	set_color(shadow2);
  	draw_line(x2, y1, x2, y3);
  	draw_line(x2, y3, x1, y2);

// top left
	set_color(light1);
	draw_line(x1, y2, x2, y1);
	set_color(light2);
	draw_line(x1, y2+1, x2, y1+1);
return 0;
}

int BC_Tool::draw_triangle_right(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
  	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	y1 = y; y2 = y + h / 2; y3 = y + h - 1; 
	x1 = x; x2 = x + w - 1;

	point[0].x = x1; point[0].y = y1; point[1].x = x2; 
	point[1].y = y2; point[2].x = x1; point[2].y = y3;

	set_color(middle);
  	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// left and top right
	set_color(light2);
	draw_line(x1+1, y3, x1+1, y1);
	draw_line(x1, y1+1, x2, y2+1);
	set_color(light1);
	draw_line(x1, y3, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow1);
  	draw_line(x2, y2-1, x1, y3-1);
	set_color(shadow2);
  	draw_line(x2, y2, x1, y3);
return 0;
}

int BC_Tool::draw_triangle_left(int x, int y, int w, int h, 
	int light, int middle, int shadow)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; y1 = y; x2 = x + w / 2;
	y2 = y + h - 1; x3 = x + w - 1;

// middle
	point[0].x = x2; point[0].y = y1; point[1].x = x3;
	point[1].y = y2; point[2].x = x1; point[2].y = y2;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// bottom and top right
	set_color(shadow);
	draw_line(x3, y2, x1, y2);
	draw_line(x2, y1, x3, y2);

// top left
	set_color(light);
	draw_line(x2, y1, x1, y2);
return 0;
}

int BC_Tool::draw_triangle_right(int x, int y, int w, int h, 
	int light, int middle, int shadow)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; x2 = x + w / 2; x3 = x + w - 1;
	y1 = y; y2 = y + h - 1;

	point[0].x = x2; point[0].y = y2; point[1].x = x3;
	point[1].y = y1; point[2].x = x1; point[2].y = y1;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// top and bottom left
	set_color(light);
	draw_line(x3, y1, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow);
	draw_line(x3, y1, x2, y2);
return 0;
}



int BC_Tool::draw_3d_circle(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2)
{
// clear circle area
	set_color(middle);

	XFillArc(top_level->display, pixmap, top_level->gc, x, y, w-1, h-2, 0*64, 360*64);

// draw upper left arc
	set_color(light1);
	XDrawArc(top_level->display, pixmap, top_level->gc, x, y, w-1, h-2, 45*64, 180*64);
	set_color(light2);
	XDrawArc(top_level->display, pixmap, top_level->gc, x+1, y+1, w-3, h-4, 45*64, 180*64);

// draw lower right arc
	set_color(shadow1);
	XDrawArc(top_level->display, pixmap, top_level->gc, x+1, y+1, w-3, h-4, 225*64, 180*64);
	set_color(shadow2);
	XDrawArc(top_level->display, pixmap, top_level->gc, x, y, w-1, h-2, 225*64, 180*64);
return 0;
}

int BC_Tool::draw_box_colored(int x, int y, int w, int h, int down, int highlighted)
{
	if(!down)
	{
		if(highlighted)
			draw_3d_big(x, y, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_shadow,
				BLACK);
		else
			draw_3d_big(x, y, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_shadow,
				BLACK);
	}
	else
	{
// need highlighting for toggles
		if(highlighted)
			draw_3d_big(x, y, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_up,
				top_level->get_resources()->button_light);
		else
			draw_3d_big(x, y, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				top_level->get_resources()->button_down, 
				top_level->get_resources()->button_down,
				top_level->get_resources()->button_light);
	}
return 0;
}

int BC_Tool::draw_3d_small(int x1, int y1, int w, int h, 
	int light, int middle, int shadow)
{
	h--; w--;
	
	set_color(middle);
	draw_box(x1, y1, w, h);

	set_color(light);
	draw_line(x1, y1, x1+w, y1);
	draw_line(x1, y1, x1, y1+h);

	set_color(shadow);
	draw_line(x1+w, y1, x1+w, y1+h);
	draw_line(x1, y1+h, x1+w, y1+h);
return 0;
}

int BC_Tool::draw_rectangle(int x_, int y_, int w_, int h_) 
{ 
	XDrawRectangle(top_level->display, pixmap, top_level->gc, x_, y_, w_ - 1, h_ - 1); 
return 0;
}

int BC_Tool::draw_box(int x_, int y_, int w_, int h_) 
{ 
  //printf("draw_box %d\n", pixmap);
	XFillRectangle(top_level->display, pixmap, top_level->gc, x_, y_, w_, h_); 
return 0;
}

int BC_Tool::draw_line(int x1, int y1, int x2, int y2) 
{
	XDrawLine(top_level->display, pixmap, top_level->gc, x1, y1, x2, y2);
return 0;
}

int BC_Tool::draw_3d_line(int x1, int y1, int x2, int y2, int color1, int color2, GC *gc)
{
	set_color(color1, gc ? gc : &(top_level->gc));
	XDrawLine(top_level->display, pixmap, gc ? *gc : top_level->gc, x1, y1, x2, y2);
	set_color(color2, gc ? gc : &(top_level->gc));
	XDrawLine(top_level->display, pixmap, gc ? *gc : top_level->gc, x1, y1+1, x2, y2+1);
return 0;
}

int BC_Tool::draw_disc(int x, int y, int w, int h, GC *gc)
{
	XFillArc(top_level->display, pixmap, gc ? *gc : top_level->gc, x, y, (w - 1), (h - 2), 0*64, 360*64);
return 0;
}

// Draw a VFrame on the screen
int BC_Tool::draw_bitmap(VFrame *frame, 
			int in_x1, int in_y1, int in_x2, int in_y2, 
			int out_x1, int out_y1, int out_x2, int out_y2, int use_alpha, GC *gc)
{
	if(!gc) gc = &(top_level->gc);

	get_temp_bitmap(&image_bitmap, out_x2 - out_x1, out_y2 - out_y1);
	image_bitmap->read_frame(frame, in_x1, in_y1, in_x2, in_y2, use_alpha);
	image_bitmap->write_drawable(pixmap, out_x1, out_y1, 0, 0, out_x2 - out_x1, out_y2 - out_y1, 0);
return 0;
}

// Draw a bitmap on the screen
int BC_Tool::draw_bitmap(BC_Bitmap *bitmap, 
			int fast, 
			int dont_wait, 
			GC *gc)
{
	int new_w, new_h;

	if(!gc) gc = &(top_level->gc);

	new_w = bitmap->w < w ? bitmap->w : w;
	new_h = bitmap->h < h ? bitmap->h : h;

// Use hardware scaling to canvas dimensions if proper color model.
	if(bitmap->depth == BC_YUV420P)
	{
		new_w = w;
		new_h = h;
	}

	if(fast)
	{
		bitmap->write_drawable(win, 0, 0, 0, 0, 
			new_w, new_h, dont_wait);

		XFlush(top_level->display);
	}
	else
	{
		bitmap->write_drawable(pixmap, 0, 0, 0, 0, 
			new_w, new_h, dont_wait);
	}
	return 0;
return 0;
}

int BC_Tool::draw_text(int x_, int y_, char *text) 
{
	XDrawString(top_level->display, pixmap, top_level->gc, x_, y_, text, strlen(text));
return 0;
}

int BC_Tool::draw_vertical_text(int x, int y, char *text, int fgcolor, int bgcolor, GC *gc)
{
	int len = strlen(text);
	int w = get_text_width(MEDIUMFONT, text) + 10;
	int h = get_text_height(MEDIUMFONT);
	int d = w > h ? w : h;

	if(!gc) gc = &(top_level->gc);

	get_font_pixmap(d, d);     // pixmap has to be the same size as bitmap for ShmGetImage
	get_temp_bitmap(&font_bitmap, d, d);
	set_color(bgcolor, gc);
	XFillRectangle(top_level->display, font_pixmap, *gc, 0, 0, d, d);
	set_color(fgcolor, gc);
	XDrawString(top_level->display, font_pixmap, *gc, 0, get_text_ascent(MEDIUMFONT), text, len);
	font_bitmap->read_drawable(font_pixmap, 0, 0);
//printf("BC_Tool::draw_vertical_text 8 %x\n", font_bitmap);
	font_bitmap->rotate_90(d);
//printf("BC_Tool::draw_vertical_text 9\n");
	font_bitmap->write_drawable(pixmap, x, y, d - h, 0, h, w, 0);
//printf("BC_Tool::draw_vertical_text 10\n");
return 0;
}



int BC_Tool::draw_center_text(int x, int y, char *text, int font, GC *gc)
{
	if(!gc) gc = &(top_level->gc);
	x -= get_text_width(font, text) / 2;
	XDrawString(top_level->display, pixmap, *gc, x, y, text, strlen(text));
return 0;
};

int BC_Tool::get_text_height(int font)
{
	return get_text_ascent(font) + get_text_descent(font);
return 0;
}

int BC_Tool::get_text_ascent(int font)
{
	return top_level->get_font(font)->ascent;
return 0;
}

int BC_Tool::get_text_descent(int font)
{
	return top_level->get_font(font)->descent;
return 0;
}

int BC_Tool::set_font(int font) 
{
	top_level->set_font(font);
return 0;
};

int BC_Tool::get_text_width(int font, char *text)
{
	return XTextWidth(top_level->get_font(font), text, strlen(text));
return 0;
}

int BC_Tool::slide_left(int distance)
{
	if(distance < w)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, distance, 0, w - distance, h, 0, 0);
	}
return 0;
}

int BC_Tool::slide_right(int distance)
{
	if(distance < w)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, 0, w - distance, h, distance, 0);
	}
return 0;
}

int BC_Tool::slide_up(int distance)
{
	if(distance < h)
	{
	  XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, distance, w, h - distance, 0, 0);
		set_color(subwindow->get_color());
	  XFillRectangle(top_level->display, pixmap, top_level->gc, 0, h - distance, w, distance);
  }
return 0;
}

int BC_Tool::slide_down(int distance)
{
	if(distance < h)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, 0, w, h - distance, 0, distance);
		set_color(subwindow->get_color());
		XFillRectangle(top_level->display, pixmap, top_level->gc, 0, 0, w, distance);
	}
return 0;
}

int BC_Tool::uses_text()
{             // set to 1 if tool uses text input
	return 0;
return 0;
}

int BC_Tool::get_font_pixmap(int w, int h)
{
	if(font_pixmap && (w > font_pixmap_w || h > font_pixmap_h))
	{
		XFreePixmap(top_level->display, font_pixmap);
		font_pixmap = 0;
	}

	if(!font_pixmap)
	{
		font_pixmap = XCreatePixmap(top_level->display, win, w, h, top_level->depth);
	}
return 0;
}

int BC_Tool::get_temp_bitmap(BC_Bitmap **bitmap, int w, int h)
{
//printf("BC_Tool::get_font_bitmap 1\n");
// the bitmap must be wholly contained in the source during a GetImage
	if(*bitmap && (w != (*bitmap)->w || h != (*bitmap)->h))
	{
//printf("BC_Tool::get_font_bitmap 2\n");
		delete *bitmap;
		*bitmap = 0;
	}

	if(!*bitmap)
	{
//printf("BC_Tool::get_font_bitmap 3\n");
		*bitmap = new BC_Bitmap(top_level, w, h, top_level->depth);
//printf("BC_Tool::get_font_bitmap 4\n");
	}
return 0;
}

Window BC_Tool::get_event_win()
{ return top_level->event_win; }

Window BC_Tool::get_top_win()
{ return top_level->win; }

BC_Tool* BC_Tool::get_active_tool()
{ return top_level->active_tool; }

int BC_Tool::get_button_down()
{ return top_level->button_down; return 0;
}

int BC_Tool::set_button_down(int value)
{ top_level->button_down = value; return 0;
}

BC_Resources* BC_Tool::get_resources()
{ return top_level->resources; }

int BC_Tool::set_active_tool(BC_Tool *tool)
{ top_level->active_tool = tool; return 0;
}

BC_MenuBar* BC_Tool::get_active_menubar()
{ return top_level->active_menubar; }

int BC_Tool::set_active_menubar(BC_MenuBar* menubar)
{ top_level->active_menubar = menubar; return 0;
}

BC_PopupMenu* BC_Tool::get_active_popupmenu()
{ return top_level->active_popup_menu; }

int BC_Tool::set_active_popupmenu(BC_PopupMenu* menu)
{ top_level->active_popup_menu = menu; return 0;
}

int BC_Tool::get_w() { return w; return 0;
}
int BC_Tool::get_h() { return h; return 0;
}
int BC_Tool::get_x() { return x; return 0;
}
int BC_Tool::get_y() { return y; return 0;
}
int BC_Tool::get_cursorx() { return cursor_x; return 0;
}
int BC_Tool::get_cursory() { return cursor_y; return 0;
}
int BC_Tool::get_color() { return color; return 0;
}

BC_ToolItem::BC_ToolItem(BC_Tool *pointer)
 : ListItem<BC_ToolItem>()
{
	this->pointer = pointer;
	pointer->list_item = this;
}

BC_ToolItem::~BC_ToolItem()
{
//printf("BC_ToolItem::~BC_ToolItem %x\n", pointer);
	if(pointer)
	{
		pointer->list_item = 0;      // stop an infinite loop
		delete pointer;        // delete the tool object
		pointer = 0;
	}
}

BC_ToolList::BC_ToolList()
 : List<BC_ToolItem>()
{
}

BC_ToolList::~BC_ToolList()
{
// delete from first to last since some tools delete their own tools
	while(first) delete first;
}

int BC_ToolList::append(BC_Tool *tool)
{
	List<BC_ToolItem>::append(new BC_ToolItem(tool));
return 0;
}

int BC_ToolList::remove(BC_Tool *tool)
{
	BC_ToolItem *current;
	
	for(current = first; current && current->pointer != tool; current = NEXT)
		;
	
	if(current) List<BC_ToolItem>::remove(current);
return 0;
}
