#ifndef BCWINDOWBASE_H
#define BCWINDOWBASE_H

#include "arraylist.h"
#include "bcbitmap.inc"
#include "bcresources.inc"
#include "bcsubwindow.inc"
#include "bctool.inc"
#include "bcwindow.inc"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

class BC_ResizeCall
{
public:
	BC_ResizeCall(int w, int h);
	int w, h;
};

class BC_WindowBase
{
public:
	BC_WindowBase() { };
	BC_WindowBase(int x, int y, int w, int h, int color = -1);
	virtual ~BC_WindowBase();

	friend BC_SubWindow;
	friend BC_Window;

// affect this window and all its subwindows

	int enable_window();         // enable all events
	int disable_window();        // disable all events except expose
	int destroy_window();        // destroy the window but keep the object
	int lock_window();           // lock the window from events
	int unlock_window();         // unlock the window from events

// ================================= initialization
	virtual int create_objects() { return 0; };    // routine user adds components from

	virtual BC_Tool* add_tool(BC_Tool *tool);    // add a lowest level component
	int add_border();         // add a border to this window
	int add_border(int light, int medium, int dark);         // add a border to this window
	int add_border(int light1, int light2, int medium, int dark1, int dark2);// add a border to this window
	BC_SubWindow* add_subwindow(BC_SubWindow* subwindow);         // add a subwindow object
	int delete_tool(BC_Tool *tool);                   // delete a lowest level component
	int delete_subwindow(BC_SubWindow* subwindow);           // delete a subwindow object
	int resize_window(int x, int y, int w, int h);       // user calls to resize this window
	int resize_window(int w, int h);       // user calls to resize this window

// ================================= user declared event handlers

	virtual int cursor_motion() { return 0; };
	virtual int button_release() { return 0; };
	virtual int button_press() { return 0; };
	virtual int resize_event(int w, int h) { return 0; };      // gives w and h of parent window

// ============================ internal event dispatchers

	int keypress_event_dispatch();
	int expose_event_dispatch();
	int button_press_dispatch();
	int button_release_dispatch();
	int motion_event_dispatch();
	int resize_event_dispatch();
	int unhighlight();                // unhighlight all tools
	int flash();
	virtual int repeat_event_dispatch(long repeat_id);          // for repeats
	int cursor_left_dispatch();

// =============================== queries

	int find_next_textbox(BC_Tool **tool, int *result);        // return the next textbox in line
	int find_first_textbox(BC_Tool **tool);                //  find a textbox
	int get_button_down();   // Whether or not the button is down
	int get_buttonpress();   // Button pressed if this is a button press event

// ======================================= drawing commands

	int draw_border();
	int draw_line(int x1, int y1, int x2, int y2);
	int set_color(int color);
	int get_w();
	int get_h();
	int get_x();
	int get_y();
	int get_color();
	int get_cursor_x();
	int get_cursor_y();
	int get_keypress();
	int get_key_pressed();
// If a yuv colormodel in bcbitmap.h is available
	int colormodel_available(int color_model); 
	BC_Resources* get_resources();

 	Window win;               // this window

// parameters for all

	int cursor_x, cursor_y;        // cursor position of last event relative to this window
	int enabled;                  // if this window is enabled

	BC_Window *top_level;           // pointer to top level window or this if it is the top
	BC_WindowBase *parent_window;   // pointer to parent window or this if it is the top
	BC_SubWindowList* subwindows;     // list of subwindows owned by this window
	BC_ToolList* tools;              // list of lowest level components owned
private:
	int border;                        // if this window has a border
	int light1, light2, medium, dark1, dark2;           // border colors
	int x, y, w, h, color;          // position of subwindow or size of top level window
	ArrayList<BC_ResizeCall*> resize_history;
};




#endif
