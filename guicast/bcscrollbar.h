#ifndef BCSCROLLBAR_H
#define BCSCROLLBAR_H

#include "bclistbox.inc"
#include "bcsubwindow.h"

// Orientations
#define SCROLL_HORIZ 0
#define SCROLL_VERT  1

#define SCROLL_HANDLE 1
#define SCROLL_BACKPAGE 2
#define SCROLL_FWDPAGE 3
#define SCROLL_BACKARROW 4
#define SCROLL_FWDARROW 5

#define SCROLL_SPAN 17
#define SCROLL_MARGIN 2
#define MINHANDLE 10

class BC_ScrollBar : public BC_SubWindow
{
public:
	BC_ScrollBar(int x, 
		int y, 
		int orientation, 
		int pixels, 
		long length, 
		long position, 
		long handlelength);
	virtual ~BC_ScrollBar();

	friend BC_ListBox;

	virtual int handle_event() { return 0; };
	int initialize();
	int cursor_motion_event();
	int cursor_leave_event();
	int cursor_enter_event();
	int button_press_event();
	int button_release_event();
	int repeat_event(long repeat_id);
	long get_value();
	long get_position();
	long get_length();
	long get_handlelength();
	int in_use();
	int update_value(long value);
	int update_length(long length, long position, long handlelength);
	int reposition_window(int x, int y, int w = -1, int h = -1);

private:
	int activate();
	void draw();
	void get_handle_dimensions();
	int get_cursor_zone(int cursor_x, int cursor_y);

	long length, position, handlelength;   // handle position and size
	int selection_status, highlight_status;
	int orientation, pixels;
	int handle_pixel, handle_pixels;
	int min_pixel, max_pixel;
	long repeat_count;
// Don't deactivate if bound to another tool
	BC_WindowBase *bound_to;
};

#endif
