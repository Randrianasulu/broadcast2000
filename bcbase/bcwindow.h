#ifndef BCWINDOW_H
#define BCWINDOW_H

class BC_Window;
class BC_MenuBar;
class BC_PopupMenu;

#include "arraylist.h"
#include "bcbitmap.inc"
#include "bcmenubar.inc"
#include "bcpopup.inc"
#include "bcrepeater.inc"
#include "bcresources.inc"
#include "bcsubwindow.inc"
#include "bctool.inc"
#include "bcwindowbase.h"
#include "mutex.h"
#include "timer.h"

// ================================= top level windows only

class BC_Window : public BC_WindowBase
{
public:
	BC_Window() {  };

// build a standalone window
	BC_Window(char *title, 
				int w, 
				int h, 
				int minw, 
				int minh, 
				int private_color = 0, 
				int hide = 0);

	BC_Window(char *display_name, 
				int color,
				char *title,
				int w, 
				int h,
				int minw, 
				int minh,
				int private_color = 0,
				int hide = 0);
	
	virtual ~BC_Window();

	friend BC_Bitmap;
	friend BC_Popup;
	friend BC_Resources;
	friend BC_SubWindow;
	friend BC_Tool;
	friend BC_WindowBase;
	friend BC_Repeater;

// =============================== initialization

// need routines here to set top_level
	BC_Tool* add_tool(BC_Tool *tool);                     // add a lowest level component
	int add_border(int light, int medium, int dark);         // add a border to this window
	BC_SubWindow* add_subwindow(BC_SubWindow* subwindow);         // add a subwindow object

	int run_window();            // run event handler
	int hide_window();           // hide window
	int show_window();           // show window
	int lock_window();           // lock the window from events
	int unlock_window();         // unlock the window from events

// repeating events
	int set_done(int return_value);                    // quit the window event dispatcher
	int set_repeat(long duration, long id);            // signal the event handler to send a repeat after this duration
	long get_repeat(long repeat_id);                   // get repeat duration for the id
	int unset_repeat(long repeat_id);                  // tell event handler to stop sending repeats
	int unset_all_repeaters();                         // delete all repeater opjects for a close
	int repeat_event_dispatch(long repeat_id, int interrupt_now);   // dispatch a repeat event
	long new_repeat_id(); // get unique id for the next repeater
	int arm_repeat(long repeat_id, int interrupt_now);  // repeater is a unique id for the repeat thread

// window title
	int set_title(char *title);                        // set the title of this standalone window


// ================================= clipboard

	int to_clipboard(char *data);                  // send data to the global clipboard
	int clipboard_len();                           // get length of clipboard data
	int from_clipboard(char *data, int max = 1024);                // get data from the global clipboard

// ================================= user declared event handlers

	virtual int keypress_event() { return 0; };       // handle a key press
	virtual int close_event() { return 0; };        // close box pressed
	int shift_down();                                  // flags for certain keys
	int ctrl_down();




// ================================== event dispatchers

	int dispatch_event();        // send event to all subwindows until trapped
	int store_motion_event(XEvent *report);    // arm the motion buffer with the current event
	int dispatch_motion_event_main();           // master motion event dispatcher translates motion history
	int window_reset();                    // flush all the events in the queue
	int cycle_textboxes();                 // activate the next textbox in the window


// color table
// 8 bit is rrgggbbb
// 16 bit is rrrrrggggggbbbbb
	int get_color(long color);          // return the colormap pixel of the color for all bit depths
	int get_color_8(int color);         // get the pixel value for the color for 8 bit
	long get_color_16(int color);   // get the pixel value for the color for 16 bit
	long get_color_16_swap(int color);   // get the pixel value for the color for 16 bit
	long get_color_24_swap(int color);

// fonts
	XFontStruct *get_font(int font);      // get the font structure from the macro
	int set_font(int font);           // set the top level's gc to the font


// window can get destroyed after the button release and the last motion notify events in it
	int set_last_deleted(Window win);

// Eventually depth should be a colormodel but currently it's merely a number of bits
	int get_depth();
	int set_key_pressed(int new_value);
	int get_double_click();
	int get_hidden();
	int set_button_just_released();
	int get_top_w();
	int get_top_h();
	GC gc;
	Display* display;
	Window rootwin;

private:
	int init_flags();
	int init_window(char *display_name, char *title, int hide, int private_color, int minw, int minh);
	int get_atoms();
	int init_gc();
	int init_fonts();
	int init_colors();

// Wait until the event appears in the queue and take it out of the queue
// without touching anything else.
	int pick_event(int event_type);

// Get the key masks from the event.
	int get_key_masks(XEvent &report);

// Access the registry of windows in existance.
	int add_window(Window win);
	int delete_window(Window win);
	int window_exists(Window win);

// information for repeating
	long next_repeat_id; // next repeat_id for tools
// Array of repeaters for multiple repeating objects.
	ArrayList<BC_Repeater*> repeaters;

// If an extremely slow operation starts the repeater before
// finishing, a button release in the main event handler
// won't be caught until after repeat events start.
// So don't start the repeater until after the event handler finishes the operation.
	BC_Repeater* new_repeater;

// motion history buffer
	int motion_buffer;       // motion event in buffer
	int motion_buffer_x, motion_buffer_y;      // coordinates of buffered motion

// masks for event queries
	int ctrl_mask, shift_mask, key_sym, resized;
// some window managers send the same configure event over and over and over again
// which breaks some routines for flipping vertical
	int last_w, last_h;
	int done;
	unsigned long button_time1, button_time2;
	int cursorleft;
	int return_value;
// Pointer to the active menubar in the window.
// Events go to this first.
	BC_MenuBar* active_menubar;

// Pointer to the active tool in this window.
// Events go to this second.
	BC_Tool* active_tool;
	BC_PopupMenu* active_popup_menu;    // pointer to the active popup menu in the window
	int button_down;
	int button_just_released;
	int button_pressed;
	int depth;
	int key_pressed;
	int double_click;
	int hidden;

// color table
	int allocate_color_table();         // install the color table in the X server
	int create_private_colors();        // create a table containing 255 colors including the necessary colors
	int create_shared_colors();         // create a table containing only the necessary colors
	int create_color(int color);    // add the color to the table or replace the closest match with the color
	int color_table[256][2];    // table for every color allocated
	int private_color;          // use installed colormap
	int total_colors;           // number of colors in color table
	int current_color_value, current_color_pixel;// last color found in table

// X variables
// List of all the windows currently in existance.
	ArrayList<Window> window_registry;

// Temporary event queue for removing events farther down the queue.
	ArrayList<XEvent> event_queue;

	Window motion_buffer_win;       // window of buffered motion
 	Window event_win;       // window the last event happened in
	Window last_deleted_window;       // store most recent deleted window
	XFontStruct *largefont, *mediumfont, *smallfont;      // fonts used by everyone
	static BC_Resources *resources;       // color settings from window manager
	Visual *vis;
	Colormap cmap;
	Atom DelWinXAtom;
	Atom ProtoXAtom;
	Atom RepeaterXAtom;
	int screen;
// The window is supposed to be locked by one thread during ShmPutImage
// so that it can get the shm_completion but usually it isn't.
// Keep track of the number of shm_completions leaking through.
	int shm_completions;
	int client_byte_order, server_byte_order;
};

#endif
