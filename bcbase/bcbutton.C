#include "bcbutton.h"
#include "bcfont.h"
#include "bcresources.h"
#include <string.h>
BC_Button::BC_Button(int x, int y, char *text, int big)
	: BC_Tool(x, y, 0, 0)
{
	this->text = new char[strlen(text) + 1];
	this->big = big;
	strcpy(this->text, text);
	down = button_down = cursor_over = highlighted = 0;
}

BC_Button::BC_Button(int x, int y, int w, char *text, int big)
	: BC_Tool(x, y, w, 0)
{
	this->text = new char[strlen(text) + 1];
	this->big = big;
	strcpy(this->text, text);
	down = button_down = cursor_over = highlighted = 0;
}

BC_Button::~BC_Button() { delete text; }

int BC_Button::create_tool_objects() { create_window(x, y, w, h, MEGREY); return 0;
}

int BC_Button::resize_tool(int x, int y)
{
	resize(x, y);
return 0;
}

int BC_Button::resize(int x, int y)
{ 
	resize_window(x, y, w, h);
	draw(); 
return 0;
}

int BC_Button::get_down()
{ return down; return 0;
}

int BC_Button::is_big()
{ return big; return 0;
}

int BC_Button::get_highlighted()
{ return highlighted; return 0;
}

char* BC_Button::get_text()
{ return text; }

int BC_Button::set_down(int new_value)
{
	down = new_value;
return 0;
}

int BC_Button::cursor_left_()
{
	if(highlighted)
	{
		if(get_cursor_x() < 0 || get_cursor_x() > w ||
			 get_cursor_y() < 0 || get_cursor_y() > h)
		{   // draw unhighlighted
			highlighted = 0;
			draw();
		}
	}
return 0;
}

int BC_Button::button_press_()
{
	if(get_event_win() != get_top_win()) return 0;
	if(  // subwindow test
		subwindow->get_cursor_x() > 0 && subwindow->get_cursor_x() < subwindow->get_w()
			&& subwindow->get_cursor_y() > 0 && subwindow->get_cursor_y() < subwindow->get_h()
		&& // button test
		get_cursor_x() > 0 && get_cursor_x() < w
			&& get_cursor_y() > 0 && get_cursor_y() < h)
	{
		down = button_down = cursor_over = 1;
		draw();
		if(get_active_tool()) get_active_tool()->deactivate();
		button_press();
		return 1;
	}
	return 0;
return 0;
}

int BC_Button::cursor_motion_()
{
	int result;
	result = 0;

	if(button_down)
	{
		if(cursor_over)
		{
			if(get_cursor_x() < 0 || get_cursor_x() > w ||
				 get_cursor_y() < 0 || get_cursor_y() > h)
			{
				down = 0;
				cursor_over = 0;
				highlighted = 0;
				draw();
			}
		}
		else
		{
			if(get_cursor_x() > 0 && get_cursor_x() < w &&
				 get_cursor_y() > 0 && get_cursor_y() < h)
			{
				down = 1;
				cursor_over = 1;
				top_level->unhighlight();
				highlighted = 1;
				draw();
				result = 1;
			}
		}
	}
	else
	{
  		if(get_button_down()) return 0;
  		if(get_event_win() != get_top_win()) return 0;

		if(get_cursor_x() > 0 && get_cursor_x() < w &&
			 get_cursor_y() > 0 && get_cursor_y() < h)
		{
			result = 1;
			if(!highlighted)
			{   // draw highlighted
				top_level->unhighlight();
				highlighted = 1;
				draw();
			}
		}
		else
		if(get_cursor_x() < 0 || get_cursor_x() > w ||
			 get_cursor_y() < 0 || get_cursor_y() > h)
		{
			if(highlighted)
			{   // draw unhighlighted
				highlighted = 0;
				draw();
			}
		}
	}
	return result;
return 0;
}

int BC_Button::repeat_()
{
	if(down) return repeat();     // go to user
	else return 0;
return 0;
}

int BC_Button::button_release_()
{
	if(button_down)
	{
		if(down)
		{
			down = 0;
			handle_event();      // call user event handler
			draw();
		}
		button_release();   // call extra user handler for button events only
		button_down = 0;
		return 1;
	}
	return 0;
return 0;
}

int BC_Button::keypress_event_() { return keypress_event(); return 0;
}

int BC_Button::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		draw();
	}
	return 0;
return 0;
}


int BC_Button::update(char *text)
{
	delete this->text;
	this->text = new char[strlen(text) + 1];
	strcpy(this->text, text);
	draw();
return 0;
}

int BC_Button::draw_small_box()
{
	if(!get_down())
	{
		if(get_highlighted())
			draw_3d_small(0, 0, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_shadow);
		else
			draw_3d_small(0, 0, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_shadow);
	}
	else
	{
// need highlighting for toggles
		//if(get_highlighted())
		//	draw_3d_small(0, 0, w, h, 
		//		top_level->get_resources()->button_shadow, 
		//		top_level->get_resources()->button_up,
		//		top_level->get_resources()->button_light);
		//else
			draw_3d_small(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				top_level->get_resources()->button_down, 
				top_level->get_resources()->button_light);
	}
return 0;
}

// ===================================================================

BC_BigButton::BC_BigButton(int x, int y, char *text, int big)
 : BC_Button(x, y, text, big) { }

int BC_BigButton::create_tool_objects()
{
	h = 25; w = get_text_width(MEDIUMFONT, get_text()) + 25;
	create_window(x, y, w, h, MEGREY);
	draw();
return 0;
}

int BC_BigButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	//set_color(BLACK);
	//draw_rectangle(0, 0, w, h);
	set_color(get_resources()->text_default);
	set_font(MEDIUMFONT);
	draw_center_text(w / 2, h - 7, get_text(), MEDIUMFONT);
	flash();
return 0;
}

BC_SmallButton::BC_SmallButton(int x, int y, char *text, int big)
 : BC_Button(x, y, text, big) { }

BC_SmallButton::BC_SmallButton(int x, int y, int w, char *text, int big)
 : BC_Button(x, y, w, text, big) { }

int BC_SmallButton::create_tool_objects()
{
	h = 20; 
	if(w == 0) w = get_text_width(MEDIUMFONT, get_text()) + 20;
	create_window(x, y, w, h, MEGREY);
	draw();
return 0;
}

int BC_SmallButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	set_color(get_resources()->text_default);
	set_font(MEDIUMFONT);
	draw_center_text(w / 2, h - 5, get_text(), MEDIUMFONT);
	flash();
return 0;
}

BC_UpTriangleButton::BC_UpTriangleButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_UpTriangleButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_UpTriangleButton::draw()
{
	if(get_down())
	{
		if(is_big())
		{
			draw_triangle_up(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				get_resources()->button_down,
				get_resources()->button_down,
				get_resources()->button_light);
		}
		else
			draw_triangle_up(0, 0, w, h, 
				get_resources()->button_shadow,
				get_resources()->button_down,
				get_resources()->button_light);
	}
	else
	{
  		if(get_highlighted())
		{
			if(is_big())
				draw_triangle_up(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_highlighted,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_up(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_shadow);
		}
  		else
		{
			if(is_big())
				draw_triangle_up(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_up,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_up(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_shadow);
		}
	}
	flash();
return 0;
}

BC_DownTriangleButton::BC_DownTriangleButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_DownTriangleButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_DownTriangleButton::draw()
{
	if(get_down())
	{
		if(is_big())		
			draw_triangle_down(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				get_resources()->button_down,
				get_resources()->button_down,
				get_resources()->button_light);
		else
			draw_triangle_down(0, 0, w, h, 
				get_resources()->button_shadow,
				get_resources()->button_down,
				get_resources()->button_light);
	}
	else
	{
  		if(get_highlighted())
		{
			if(is_big())
				draw_triangle_down(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_highlighted,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_down(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_light,
					get_resources()->button_shadow);
		}
  		else
		{
			if(is_big())
				draw_triangle_down(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_up,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_down(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_shadow);
		}
	}
	flash();
return 0;
}


BC_LeftTriangleButton::BC_LeftTriangleButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_LeftTriangleButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_LeftTriangleButton::draw()
{
	if(get_down())
	{
		if(is_big())
		{
			draw_triangle_left(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				get_resources()->button_down,
				get_resources()->button_down,
				get_resources()->button_light);
		}
		else
			draw_triangle_left(0, 0, w, h, 
				get_resources()->button_shadow,
				get_resources()->button_down,
				get_resources()->button_light);
	}
	else
	{
  		if(get_highlighted())
		{
			if(is_big())
				draw_triangle_left(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_highlighted,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_left(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_shadow);
		}
  		else
		{
			if(is_big())
				draw_triangle_left(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_up,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_left(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_shadow);
		}
	}
	flash();
return 0;
}

BC_RightTriangleButton::BC_RightTriangleButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_RightTriangleButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_RightTriangleButton::draw()
{
	if(get_down())
	{
		if(is_big())		
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				get_resources()->button_down,
				get_resources()->button_down,
				get_resources()->button_light);
		else
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_shadow,
				get_resources()->button_down,
				get_resources()->button_light);
	}
	else
	{
  		if(get_highlighted())
		{
			if(is_big())
				draw_triangle_right(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_highlighted,
					get_resources()->button_highlighted,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_right(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_light,
					get_resources()->button_shadow);
		}
  		else
		{
			if(is_big())
				draw_triangle_right(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_up,
					get_resources()->button_shadow,
					BLACK);
			else
				draw_triangle_right(0, 0, w, h, 
					get_resources()->button_light,
					get_resources()->button_up,
					get_resources()->button_shadow);
		}
	}
	flash();
return 0;
}

BC_RecButton::BC_RecButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_RecButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_RecButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	
	set_color(RED);
 	XFillArc(top_level->display, pixmap, top_level->gc, w / 5, w / 5, w - 2 * w / 5, h - 2 * h / 5, 0 * 64, 360 * 64);
	flash();
return 0;
}

BC_FrameRecButton::BC_FrameRecButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }     // default to play

int BC_FrameRecButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_FrameRecButton::draw()
{
	XPoint point[3];
	int x1, x2, y1, y2;
	x2 = w - w / 5; x1 = w / 5;
	y1 = h / 5; y2 = h - h / 5;

	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	set_color(RED);

	XFillRectangle(top_level->display, pixmap, top_level->gc, x1, y1, w / 5, y2 - y1);
 	XFillArc(top_level->display, pixmap, top_level->gc, w / 3, w / 4, w - 2 * w / 4, h - 2 * h / 4, 0 * 64, 360 * 64);
	flash();
return 0;
}





BC_DuplexButton::BC_DuplexButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_DuplexButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_DuplexButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();


// draw record

	set_color(RED);
  XFillArc(top_level->display, pixmap, top_level->gc, w / 6, h / 5, w / 2 - w / 6, h - h / 5 - h / 5, 180*64, 360*64);

// draw carrot

  XPoint point[3];

	set_color(GREEN);
  point[0].x = w / 2; point[0].y = h / 5; point[1].x = w / 2;
  point[1].y = h - h / 5; point[2].x = w - w / 6; point[2].y = h / 2;

  XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
	flash();
return 0;
}

// ======================================= playback buttons

// this is a base class used by all play buttons and should not by used
BC_PlayButton::BC_PlayButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{
	this->h = h; this->w = w; orange = 0;
}

int BC_PlayButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_PlayButton::update(int value)
{
	mode = value;
	draw();
return 0;
}

int BC_PlayButton::get_mode()
{
	return mode;
return 0;
}



int BC_PlayButton::set_orange(int value)
{
	this->orange = value;
return 0;
}

int BC_PlayButton::reset_button()
{
	if(orange) set_orange(0);
	update(1); 
return 0;
}

int BC_PlayButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, (get_down() || !mode), get_highlighted() && !(get_down() || !mode));
	else
		if(!(get_down() || !mode))
		{
// button up or not paused
			if(get_highlighted())
				draw_3d_small(0, 0, w, h, 
					top_level->get_resources()->button_light, 
					top_level->get_resources()->button_highlighted, 
					top_level->get_resources()->button_shadow);
			else
				draw_3d_small(0, 0, w, h, 
					top_level->get_resources()->button_light, 
					top_level->get_resources()->button_up, 
					top_level->get_resources()->button_shadow);
		}
		else
		{
// need highlighting for toggles
// button down or paused
			//if(get_highlighted())
			//	draw_3d_small(0, 0, w, h, 
			//		top_level->get_resources()->button_shadow, 
			//		top_level->get_resources()->button_up,
			//		top_level->get_resources()->button_light);
			//else
				draw_3d_small(0, 0, w, h, 
					top_level->get_resources()->button_shadow, 
					top_level->get_resources()->button_down, 
					top_level->get_resources()->button_light);
		}


// play mode
	if(mode == 1)
	{    // draw polygon
		draw_polygon();
	}
	else 
// paused
	{           // draw pause
		set_color(BLACK);
  		XFillRectangle(top_level->display, pixmap, top_level->gc, w / 5, h / 5, w / 5, h - 2 * h / 5);
  		XFillRectangle(top_level->display, pixmap, top_level->gc, 3 * w / 5, h / 5, w / 5, h - 2 * h / 5);
	}
	flash();
return 0;
}






BC_ForwardButton::BC_ForwardButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)
{ mode = 1; }     // default to play

int BC_ForwardButton::draw_polygon()
{
	XPoint point[3];

	set_color(GREEN);
	point[0].x = w / 5; point[0].y = h / 5; point[1].x = w / 5;
	point[1].y = h - h / 5; point[2].x = w - w / 5; point[2].y = h / 2;

	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
return 0;
}

BC_FrameForwardButton::BC_FrameForwardButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)
{ mode = 1; }     // default to play

int BC_FrameForwardButton::draw_polygon()
{
	XPoint point[3];
	int x1, x2, y1, y2;
	x2 = w - w / 5; x1 = w / 5;
	y1 = h / 5; y2 = h - h / 5;

	set_color(BLACK);
	point[0].x = x1 + w / 4; point[0].y = y1; point[1].x = x1 + w / 4;
	point[1].y = y2; point[2].x = x2; point[2].y = h / 2;

	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
	XFillRectangle(top_level->display, pixmap, top_level->gc, x1, y1, w / 5, y2 - y1);
return 0;
}



BC_FastForwardButton::BC_FastForwardButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)
{ mode = 1; }     // default to fast forward

int BC_FastForwardButton::draw_polygon()
{
	int x1, x2, x3, y1, y2, y3;
	XPoint point[3];

	x1 = w / 5; x2 = w / 2; x3 = w - w / 5;
	y1 = h / 5; y2 = h - h / 5; y3 = h / 2;

	point[0].x = x1; point[0].y = y1; point[1].x = x1;
	point[1].y = y2; point[2].x = x2; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

	point[0].x = x2; point[0].y = y1; point[1].x = x2;
	point[1].y = y2; point[2].x = x3; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
return 0;
}







BC_ReverseButton::BC_ReverseButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)
{ mode = 1; }     // default to fast forward

int BC_ReverseButton::draw_polygon()
{
	XPoint point[3];

	point[0].x = w - w / 5; point[0].y = h / 5; point[1].x = w / 5;
	point[1].y = h / 2; point[2].x = w - w / 5; point[2].y = h - h / 5;

	set_color(GREEN);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
return 0;
}


BC_FrameReverseButton::BC_FrameReverseButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)
{ mode = 1; }     // default to fast forward

int BC_FrameReverseButton::draw_polygon()
{
	XPoint point[3];
	int x1, x2, y1, y2;
	x2 = w - w / 5; x1 = w / 5;
	y1 = h / 5; y2 = h - h / 5;

	point[0].x = x2 - w / 4; point[0].y = y1; point[1].x = x1;
	point[1].y = h / 2; point[2].x = x2 - w / 4; point[2].y = y2;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
	XFillRectangle(top_level->display, pixmap, top_level->gc, x2 - w / 5, y1, w / 5, y2 - y1);
return 0;
}







BC_FastReverseButton::BC_FastReverseButton(int x, int y, int w, int h, int big)
 : BC_PlayButton(x, y, w, h, big)      // Fast play button
{ mode = 1; }

int BC_FastReverseButton::draw_polygon()
{
	int x1, x2, x3, y1, y2, y3;
	XPoint point[3];

	x3 = w / 5; x2 = w / 2; x1 = w - w / 5;
	y1 = h / 5; y2 = h - h / 5; y3 = h / 2;

	point[0].x = x1; point[0].y = y1; point[1].x = x1;
	point[1].y = y2; point[2].x = x2; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

	point[0].x = x2; point[0].y = y1; point[1].x = x2;
	point[1].y = y2; point[2].x = x3; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
return 0;
}








BC_StopButton::BC_StopButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{
	this->h = h; this->w = w;
}

int BC_StopButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_StopButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	set_color(BLACK);
	XFillRectangle(top_level->display, pixmap, top_level->gc, w / 5, h / 5, w - 2 * w / 5, h - 2 * h / 5);
	flash();
return 0;
}

BC_RewindButton::BC_RewindButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{ this->h = h; this->w = w; }

int BC_RewindButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_RewindButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	int x1, x2, y1, y2, y3;
	XPoint point[3];

	x1 = 2 * (w / 5); x2 = w - w / 5;
	y1 = h / 5; y2 = h - h / 5; y3 = h / 2;

	point[0].x = x2; point[0].y = y1; point[1].x = x2;
	point[1].y = y2; point[2].x = x1; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
	XFillRectangle(top_level->display, pixmap, top_level->gc, w / 5, y1, w / 5, y2 - y1);
	flash();
return 0;
}

BC_EndButton::BC_EndButton(int x, int y, int w, int h, int big)
 : BC_Button(x, y, "", big)
{
	this->h = h; this->w = w;
}

int BC_EndButton::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_EndButton::draw()
{
	if(is_big())
		draw_box_colored(0, 0, w, h, get_down(), get_highlighted() && !get_down());
	else
		draw_small_box();

	int x1, x2, y1, y2, y3;
	XPoint point[3];

	x1 = w / 5; x2 = w - 2 * (w / 5);
	y1 = h / 5; y2 = h - h / 5; y3 = h / 2;

	point[0].x = x1; point[0].y = y1; point[1].x = x1;
	point[1].y = y2; point[2].x = x2; point[2].y = y3;

	set_color(BLACK);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
	XFillRectangle(top_level->display, pixmap, top_level->gc, x2, y1, w / 5, y2 - y1);
	flash();
return 0;
}
