#ifndef BCSCROLLBAR_H
#define BCSCROLLBAR_H

class BC_ScrollBar;
class BC_XScrollBar;
class BC_YScrollBar;

#include "bccolors.h"
#include "bckeys.h"
#include "bctool.h"
#include "bcwindow.h"

// Recommended pixel size for a scrollbar.
#define BC_SCROLLBAR_SIZE  17

class BC_ScrollBar : public BC_Tool      // scrollbar base class
{
public:
	BC_ScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_);
	virtual ~BC_ScrollBar() { };

	friend BC_XScrollBar;
	friend BC_YScrollBar;

	int create_tool_objects();
	
	virtual int cursor_left_() { return 0; };
	virtual int button_press_() { return 0; };
	virtual int cursor_motion_() { return 0; };
	int button_release_();
	virtual int test_highlight() { return 0; };
	int unhighlight_();

// redraw everything
	int resize(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_);

// resize the tool
	int set_size(int x_, int y_, int w_, int h_);

// rescale scroll bar handles
	int set_position(long length, long position, long handlelength);
	int set_position(long position);

	long get_position();        // position of scrollbar
	long get_distance();        // get distance moved
	long get_length();          // get total length of scrollbar
	int in_use();                   // scrollbar is in use





// ==================================================================	

// handle resize call from subwindow
	int resize(int w, int h);       

	int get_positions(int boxlength);  // get positions for the generic box
	virtual int draw() = 0;
	int handle_arrows(int change_repeat = 1); // subset of events for arrows
	int repeat_();       // for handle_arrows

private:
// ========================================== data
	long length, position, handlelength;   // handle position and size
	long distance;
	int buttondown;                 // if button is down
	int backarrow, forwardarrow, box, backpage, forwardpage; // which is active
	int backhi, forwardhi, boxhi;     // which is highlighted
	int boxlength, handlew, handlex;  // pixel dimensions
	long init_repeat, sustain_repeat, next_repeat;      // give scrollbar a delay when first pressed
};

class BC_XScrollBar : public BC_ScrollBar
{
public:
	BC_XScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_);

	int cursor_left_();
	int button_press_();
	int cursor_motion_();

	int test_highlight();
	int draw();
	int relativex, oldx;
	long oldposition;
	int result;
};

class BC_YScrollBar : public BC_ScrollBar
{
public:
	BC_YScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_);

// return 1 if the position has changed
	int cursor_left_();
	int button_press_();
	int cursor_motion_();

	int test_highlight();
	int draw();
	
	int relativey, oldy;
	long oldposition;
	int result;
};

#endif
