#include <string.h>
#include "bcbitmap.h"
#include "bccanvas.h"
#include "bcfont.h"
#include "bcresources.h"
#include "bcwindow.h"

BC_Canvas::BC_Canvas(int x, int y, int w, int h, int color)
 : BC_Tool(x, y, w, h)
{
	this->color = color;
	video_on = 0;
}

int BC_Canvas::create_tool_objects()
{
	if(color == -1) color = top_level->get_resources()->get_bg_color();
	create_window(x, y, w, h, color);
	XGCValues gcvalues;
	unsigned long gcmask;

	gcmask = GCFont;
	gcvalues.font = top_level->get_font(MEDIUMFONT)->fid;
	gc = XCreateGC(top_level->display, win, gcmask, &gcvalues);
return 0;
}

int BC_Canvas::start_video() { video_on = 1; return 0;
}

int BC_Canvas::stop_video() { video_on = 0; return 0;
}

int BC_Canvas::video_is_on() { return video_on; return 0;
}

int BC_Canvas::set_size(int x, int y, int w, int h)
{
	this->x = x; this->y = y; this->w = w; this->h = h;
	resize_window(x, y, w, h);    // canvases must be redrawn after resize
return 0;
}

int BC_Canvas::clear_box(int x_, int y_, int w_, int h_) 
{ 
	set_color(color);
	draw_box(x_, y_, w_, h_); 
return 0;
}

int BC_Canvas::deactivate_tools()
{
	if(get_active_tool()) get_active_tool()->deactivate();
return 0;
}

int BC_Canvas::draw()
{
return 0;
}

int BC_Canvas::button_press_()
{
	int result;
	result = 0;
	if(get_cursor_x() > 0 && get_cursor_x() < get_w()
		 && get_cursor_y() > 0 && get_cursor_y() < get_h())
	{
		result = button_press();
		if(result) activate();
	}
	return result;
return 0;
}

int BC_Canvas::button_release_()
{
	button_release();
return 0;
}


int BC_Canvas::cursor_motion_()
{
	return cursor_motion();  // Only user gets it here.
return 0;
}

BC_Bitmap* BC_Canvas::new_bitmap(int w, int h, int color_model)
{
	if(color_model < 0) color_model = top_level->get_depth();
	return new BC_Bitmap(top_level, w, h, color_model);
}

int BC_Canvas::draw_bitmap(VFrame *frame, 
			int in_x1, int in_y1, int in_x2, int in_y2, 
			int out_x1, int out_y1, int out_x2, int out_y2, int use_alpha)
{
	BC_Tool::draw_bitmap(frame, 
		in_x1, in_y1, in_x2, in_y2, 
		out_x1, out_y1, out_x2, out_y2, use_alpha, &gc);
return 0;
}

int BC_Canvas::draw_bitmap(BC_Bitmap *bitmap, int dont_wait)
{
	if(video_on)
	{
		BC_Tool::draw_bitmap(bitmap, 1, dont_wait, &gc);
	}
	else
	{
		BC_Tool::draw_bitmap(bitmap, 0, dont_wait, &gc);
	}
return 0;
}

int BC_Canvas::draw_edit(int x, int y, int h) // draw edit bar
{
	set_inverse();
	set_color(RED);
	draw_line(x, y, x, y + h);
	set_opaque();
return 0;
}

int BC_Canvas::draw_start_edit(int x, int y, int vertical) // draw edit handle
{
	set_inverse();
	set_color(LTBLUE);
	XPoint point[3];
	
	if(vertical)
	{
		int x2, x3, y2;
		const int w = 15, h = 10;
		
		y2 = y + h;
		x -= w / 2; x2 = x + w / 2; x3 = x + w;

		point[0].x = x; point[0].y = y2;
		point[1].x = x2; point[1].y = y;
		point[2].x = x3; point[2].y = y2;
	}
	else
	{
		int x2, y2, y3;
		const int w = 10, h = 15;
		x2 = x + w;
		y -= h / 2; y2 = y + h / 2; y3 = y + h;

		point[0].x = x; point[0].y = y2;
		point[1].x = x2; point[1].y = y;
		point[2].x = x2; point[2].y = y3;
	}
	
	XFillPolygon(top_level->display, pixmap, gc, point, 3, Nonconvex, CoordModeOrigin);
	set_opaque();
return 0;
}

int BC_Canvas::draw_end_edit(int x, int y, int vertical) // draw edit handle
{
	set_inverse();
	set_color(LTBLUE);
	XPoint point[3];

	if(vertical)
	{
		int x2, x3, y2;
		const int w = 15, h = 10;
		y2 = y;  y -= h;
		x -= w / 2; x2 = x + w / 2; x3 = x + w;
		
		point[0].x = x; point[0].y = y;
		point[1].x = x2; point[1].y = y2;
		point[2].x = x3; point[2].y = y;		
	}
	else
	{
		int x2, y2, y3;
		const int w = 10, h = 15;	
		x2 = x; x -= w;
		y -= h / 2; y2 = y + h / 2; y3 = y + h;

		point[0].x = x; point[0].y = y;
		point[1].x = x2; point[1].y = y2;
		point[2].x = x; point[2].y = y3;
	}
	
	XFillPolygon(top_level->display, pixmap, gc, point, 3, Nonconvex, CoordModeOrigin);
	set_opaque();
return 0;
}

int BC_Canvas::draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow)
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

int BC_Canvas::set_color(int color)
{
	XSetForeground(top_level->display, gc, top_level->get_color(color)); 
return 0;
}

int BC_Canvas::set_inverse() 
{
	XSetFunction(top_level->display, gc, GXxor);
return 0;
}

int BC_Canvas::set_opaque() 
{
	XSetFunction(top_level->display, gc, GXcopy);
return 0;
}

int BC_Canvas::draw_box(int x_, int y_, int w_, int h_) 
{ 
	XFillRectangle(top_level->display, pixmap, gc, x_, y_, w_, h_); 
return 0;
}

int BC_Canvas::draw_rectangle(int x_, int y_, int w_, int h_)
{
	XDrawRectangle(top_level->display, pixmap, gc, x_, y_, w_, h_); 
return 0;
}

int BC_Canvas::draw_line(int x1, int y1, int x2, int y2) 
{
	XDrawLine(top_level->display, pixmap, gc, x1, y1, x2, y2);
return 0;
}

int BC_Canvas::draw_3d_line(int x1, int y1, int x2, int y2, int color1, int color2)
{
	BC_Tool::draw_3d_line(x1, y2, x2, y2, color1, color2, &gc);
return 0;
}

int BC_Canvas::draw_text(int x_, int y_, const char *text) 
{
	XDrawString(top_level->display, pixmap, gc, x_, y_, text, strlen(text));
return 0;
}

int BC_Canvas::draw_vertical_text(int x, int y, char *text, int fgcolor, int bgcolor)
{
	BC_Tool::draw_vertical_text(x, y, text, fgcolor, bgcolor, &gc);
return 0;
}

int BC_Canvas::draw_center_text(int x, int y, char *text, int font) 
{
	BC_Tool::draw_center_text(x, y, text, font, &gc);
return 0;
};

int BC_Canvas::set_font(int font) 
{
	XSetFont(top_level->display, gc, top_level->get_font(font)->fid);
return 0;
};

int BC_Canvas::get_text_width(int font, char *text)
{
	return BC_Tool::get_text_width(font, text);
return 0;
}

int BC_Canvas::draw_circle(int x, int y, int w, int h)
{
	XDrawArc(top_level->display, pixmap, gc, x, y, (w - 1), (h - 2), 0*64, 360*64);
return 0;
}

int BC_Canvas::draw_disc(int x, int y, int w, int h)
{
	BC_Tool::draw_disc(x, y, w, h, &gc);
return 0;
}

int BC_Canvas::flash()
{
	if(!video_on) BC_Tool::flash();
return 0;
}

int BC_Canvas::flash(int x, int y, int w, int h)
{
	if(!video_on) BC_Tool::flash(x, y, w, h);
return 0;
}
