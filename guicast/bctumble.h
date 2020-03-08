#ifndef BCTUMBLE_H
#define BCTUMBLE_H

#include "bcsubwindow.h"

class BC_Tumbler : public BC_SubWindow
{
public:
	BC_Tumbler(int x, int y);
	virtual ~BC_Tumbler();

	virtual int handle_up_event() { return 0; };
	virtual int handle_down_event() { return 0; };
	int repeat_event(long repeat_id);

	int initialize();
	int set_images(VFrame **data);
	int cursor_enter_event();
	int cursor_leave_event();
	int button_press_event();
	int button_release_event();
	int cursor_motion_event();
	int update_bitmaps(VFrame **data);
	int reposition_window(int x, int y);

private:
	int draw_face();

	BC_Pixmap *images[4];
	int status;
	long repeat_count;
};

class BC_ITumbler : public BC_Tumbler
{
public:
	BC_ITumbler(BC_TextBox *textbox, long min, long max, int x, int y);
	virtual ~BC_ITumbler();
	
	int handle_up_event();
	int handle_down_event();
	void set_boundaries(long min, long max);

	long min, max;
	BC_TextBox *textbox;
};

#endif
