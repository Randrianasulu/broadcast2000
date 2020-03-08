#ifndef BCPOPUP_H
#define BCPOPUP_H

class BC_Popup;

#include "bctool.h"
#include "bcwindow.h"

class BC_Popup          // inherited by popup windows
{
public:
// Coords are relative to top_level is top_window_coords is 1.
// Coords are relative to root otherwise.
	BC_Popup(BC_Window *top_level, int color, int x, int y, int w, int h, int top_window_coords);
// x, y, w, and h are relative to top_level window
	virtual ~BC_Popup();
	
// drawing
	int set_color(int color);
	int draw_check(int x, int y, int w, int h);
	int draw_box(int x_, int y_, int w_, int h_);
	int draw_rectangle(int x_, int y_, int w_, int h_);
	int draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow);
	int draw_3d_big(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_text(int x_, int y_, char *text);
	int draw_line(int x1, int y1, int x2, int y2);
	int get_text_width(XFontStruct *font, char *text);
	int resize_window(int x, int y, int w, int h);       // user calls to resize this window
	int flash();

// event handlers
	int motion_event_dispatch();
	int button_press_dispatch();
	int button_release_dispatch();
	int cursor_left_dispatch();
	int expose_event_dispatch();

// user event handlers
	virtual int cursor_left() { return 0; };
	virtual int cursor_motion() { return 0; };
	virtual int button_press() { return 0; };
	virtual int button_release() { return 0; };
	
	int cursor_x, cursor_y;
	Pixmap pixmap;
	Window win;
	BC_Window *top_level;
	int x, y, w, h, color;
};

#endif
