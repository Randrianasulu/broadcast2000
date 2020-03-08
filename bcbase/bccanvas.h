#ifndef BCCANVAS_H
#define BCCANVAS_H

class BC_Canvas;

#include "bccolors.h"
#include "bctool.h"
#include "bcwindow.inc"

class BC_Canvas : public BC_Tool
{
public:
	BC_Canvas(int x, int y, int w, int h, int color = -1);

	int create_tool_objects();
// ============================ user event handlers
	virtual int resize(int w, int h) { return 0; };         // user determines size here and calls set_size back
	virtual int cursor_motion() { return 0; };               // event handlers for user
	virtual int button_release() { return 0; }; 
	virtual int button_press() { return 0; };

// ============================== canvas event handlers

	int button_release_();
	int button_press_();
	int cursor_motion_();
	int set_size(int x, int y, int w, int h);    // called by user to set the size
	int deactivate_tools();            // deactivate tools if a button press
	
	virtual int draw();
// =============================== drawing 

	int start_video();     // signal not to refresh for exposes
	int stop_video();      // retain the last frame in the pixmap and enable exposes
	int video_is_on();     // Return 1 if video is on
// draw a segment of a frame anywhere on the canvas
	int draw_bitmap(VFrame *frame, 
			int in_x1, int in_y1, int in_x2, int in_y2, 
			int out_x1, int out_y1, int out_x2, int out_y2, int use_alpha);
	int draw_bitmap(BC_Bitmap *bitmap, int dont_wait);
	BC_Bitmap* new_bitmap(int w, int h, int color_model = -1);    // get a new bitmap for animation
	int draw_edit(int x, int y, int h);
	int draw_start_edit(int x, int y, int vertical);
	int draw_end_edit(int x, int y, int vertical);
	int draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow);
	int set_inverse();
	int set_opaque();
	int set_color(int color);
	int clear_box(int x_, int y_, int w_, int h_);
	int draw_box(int x_, int y_, int w_, int h_);
	int draw_rectangle(int x_, int y_, int w_, int h_);
	int draw_circle(int x_, int y_, int w_, int h_);
	int draw_disc(int x, int y, int w, int h);
	int draw_line(int x1, int y1, int x2, int y2);
	int draw_3d_line(int x1, int y1, int x2, int y2, int color1, int color2);
	int draw_text(int x_, int y_, char *text);
	int draw_vertical_text(int x, int y, char *text, int fgcolor, int bgcolor);
	int draw_center_text(int x, int y, char *text, int font);
	int get_text_width(int font, char *text);
	int set_font(int font);
	
	int flash();     // determine if video is on before flashing
	int flash(int x, int y, int w, int h);
	GC gc; // private GC for threads
	int color;
	int video_on;
};

#endif
