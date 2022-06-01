#ifndef BCTOOL_H
#define BCTOOL_H

class BC_Tool;
class BC_Tools;
class BC_ToolList;
class BC_ToolItem;

#include <X11/Xlib.h>

#include "bcbitmap.inc"
#include "bcmenubar.inc"
#include "bcpopupmenu.inc"
#include "bcresources.inc"
#include "bcwindow.inc"
#include "bcwindowbase.inc"
#include "linklist.h"
#include "vframe.inc"

class BC_Tool
{
public:
	BC_Tool(int x_, int y_, int w_, int h_);
	virtual ~BC_Tool();

	virtual int create_objects() { return 0; };           // user creates derived objects here

// ====================== user defined event handlers

	virtual int handle_event() { return 0; };
	virtual int button_press() { return 0; };
	virtual int button_release() { return 0; };
	virtual int keypress_event() { return 0; };
	virtual int repeat() { return 0; };

	int enable();
	int disable();
	int lock_window();          // lock and unlock the top window for threading
	int unlock_window();

// ====================== user settings
	int change_y(int y);      // change the value of y by this amount
	int set_y(int y);
	int set_x(int x);
	int get_keypress();          // which key was pressed
	int get_buttonpress();       // which button was pressed
	int get_double_click();
	int get_cursor_x();
	int get_cursor_y();
	int ctrl_down();
	int shift_down();
	int trap_keypress();
	int set_repeat(long repeat);
	int unset_repeat();
	long get_repeat();       // gets the repeat time for this tool if it is set
	int set_done(int return_value);       // quit the top window
	int is_active();
	
// Tool routines.
	
	int activate();     // Activate this tool by deactivating the current active_tool and pointing active_tool here
	int deactivate();   // Deactivate this tool and don't activate anything
	int create_window(int x, int y, int w, int h, int color);    // tool does this from its create_objects()
	int resize_window(int x, int y, int w, int h);        // tool calls this to resize its window
	int disable_window();        // disable and enable the top window
	int enable_window();
	int unhighlight(); // Just calls unhighlight_

// ====================== tool routines
	int create_tool_objects(BC_Window *top_level, BC_WindowBase *subwindow); // get pointers for all tools
	virtual int create_tool_objects();         // the tool's create objects routine
	virtual int flash();
	virtual int flash(int x_, int y_, int w_, int h_);
	virtual int activate_() { return 0; };        // activation routine for tool
	virtual int deactivate_() { return 0; };       // deactivate routine for tool
	virtual int unhighlight_() { return 0; };
	virtual int draw() { return 0; };

// ====================== tool event dispatch handlers
// for dispatcher
	int motion_event_dispatch();
	int cursor_left_dispatch();
	int expose_event_dispatch();
	int button_press_dispatch();
	int button_release_dispatch();
	int repeat_event_dispatch(long repeat_id);

// ======================= for tools
	virtual int cursor_left_() { return 0; };
	virtual int resize_event_(int w, int h) { return 0; };         // tool defined handler for resize from subwindow
	virtual int button_press_() { return 0; };
	virtual int cursor_motion_() { return 0; };         // for tool
	virtual int button_release_() { return 0; };
	virtual int keypress_event_() { return 0; };
	virtual int change_y_(int y) { return 0; };
	virtual int repeat_() { return 0; };

// ============================= drawing
// draw a segment of a frame anywhere on the canvas
	int draw_bitmap(VFrame *frame, 
			int in_x1, int in_y1, int in_x2, int in_y2, 
			int out_x1, int out_y1, int out_x2, int out_y2, int use_alpha, GC *gc = 0);
	int draw_bitmap(BC_Bitmap *bitmap, int fast, int dont_wait, GC *gc = 0);
	int draw_3d_diamond(int x1, int y1, int w, int h, int light, int middle, int shadow);
// hard bevel
	int draw_3d_big(int x1, int y1, int w, int h, int light, int middle, int shadow);
// Win95 style
	int draw_3d_big(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
// Border of 3D box only
	int draw_3d_border(int x, int y, int w, int h, 
		int light1, int light2, int shadow1, int shadow2);
// Win95 box with the default button colors
	int draw_box_colored(int x, int y, int w, int h, int down, int highlighted);

	int draw_triangle_up(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_triangle_down(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_triangle_down(int x, int y, int w, int h, 
		int light, int middle, int shadow);
	int draw_triangle_up(int x, int y, int w, int h, 
		int light, int middle, int shadow);
	int draw_triangle_left(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_triangle_right(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_triangle_left(int x, int y, int w, int h, 
		int light, int middle, int shadow);
	int draw_triangle_right(int x, int y, int w, int h, 
		int light, int middle, int shadow);
	int draw_3d_circle(int x, int y, int w, int h, 
		int light1, int light2, int middle, int shadow1, int shadow2);
	int draw_3d_small(int x1, int y1, int w, int h, int light, int middle, int shadow);
	int draw_center_text(int x, int y, char *text, int font, GC *gc = 0);
	int draw_text(int x_, int y_, const char *text);
	int draw_vertical_text(int x, int y, char *text, int fgcolor, int bgcolor, GC *gc = 0);
	int draw_rectangle(int x_, int y_, int w_, int h_);
	int draw_box(int x_, int y_, int w_, int h_);
	int draw_line(int x1, int y1, int x2, int y2);
	int draw_3d_line(int x1, int y1, int x2, int y2, int color1, int color2, GC *gc = 0);
	int draw_disc(int x, int y, int w, int h, GC *gc = 0);

	int set_inverse();
	int set_opaque();
	int set_color(int color, GC *gc = 0);
	int get_text_height(int font);
	int get_text_ascent(int font);
	int get_text_descent(int font);
	int set_font(int font);
	int get_text_width(int font, const char *text);
	int slide_left(int distance);
	int slide_right(int distance);
	int slide_up(int distance);
	int slide_down(int distance);
	BC_Resources* get_resources();

	int get_w();
	int get_h();
	int get_x();
	int get_y();
	int get_cursorx();
	int get_cursory();
	int get_color();

// =============== data

	virtual int uses_text();        // set to 1 if tool uses text input
	BC_Window *top_level;
	BC_WindowBase *subwindow;
	BC_ToolItem *list_item;// list item to delete when this is deleted
	Window win; 		   // tool window
	Pixmap pixmap;  	   // tool pixmap

	int color;             // color of tool background
	int x, y, w, h;        // dimensions of tool
	int cursor_x, cursor_y;// position of cursor relative to tool
	int enabled;           // Tool is on.

protected:
	Window get_event_win();
	Window get_top_win();
	BC_Tool* get_active_tool();
	int set_active_tool(BC_Tool *tool);
	BC_MenuBar* get_active_menubar();
	int set_active_menubar(BC_MenuBar* menubar);
	BC_PopupMenu* get_active_popupmenu();
	int set_active_popupmenu(BC_PopupMenu* menu);
	int get_button_down();  // If a button is down
	int set_button_down(int value);

private:
	long repeat_id;
	int get_font_pixmap(int w, int h);
	int get_temp_bitmap(BC_Bitmap **bitmap, int w, int h);   // replace the given bitmap with a new one if different sizes
	BC_Bitmap *font_bitmap, *image_bitmap;  // scratch bitmaps for rotating fonts and transferring images
	Pixmap font_pixmap;       // scratch pixmap for rotating fonts
	int font_pixmap_w, font_pixmap_h;
	int use_shm;
};

class BC_ToolItem : public ListItem<BC_ToolItem>
{
public:
	BC_ToolItem(BC_Tool *pointer);
	virtual ~BC_ToolItem();
	
	BC_Tool *pointer;
};

class BC_ToolList : public List<BC_ToolItem>
{
public:
	BC_ToolList();
	virtual ~BC_ToolList();
	
	int append(BC_Tool *tool);
	int remove(BC_Tool *tool);
};

#endif
