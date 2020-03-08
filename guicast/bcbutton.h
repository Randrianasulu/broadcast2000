#ifndef BCBUTTON_H
#define BCBUTTON_H

#include "bcbitmap.inc"
#include "bcsubwindow.h"
#include "vframe.inc"




class BC_Button : public BC_SubWindow
{
public:
	BC_Button(int x, int y, VFrame **data);
	virtual ~BC_Button();

	friend BC_GenericButton;

	virtual int handle_event() { return 0; };
	int repeat_event(long repeat_id);

	int initialize();
	virtual int set_images(VFrame **data);
	int cursor_enter_event();
	int cursor_leave_event();
	int button_press_event();
	int button_release_event();
	int cursor_motion_event();
	int update_bitmaps(VFrame **data);
	int reposition_window(int x, int y);

private:
	virtual int draw_face();

	BC_Pixmap *images[9];
	VFrame **data;
	int status;
};




class BC_GenericButton : public BC_Button
{
public:
	BC_GenericButton(int x, int y, char *text);
	int set_images(VFrame **data);
	int draw_face();

private:
	char text[BCTEXTLEN];
};

class BC_OKButton : public BC_Button
{
public:
	BC_OKButton(int x, int y);
	BC_OKButton(BC_WindowBase *parent_window);
	virtual int handle_event();
	virtual int keypress_event();
};

class BC_CancelButton : public BC_Button
{
public:
	BC_CancelButton(int x, int y);
	BC_CancelButton(BC_WindowBase *parent_window);
	virtual int handle_event();
	virtual int keypress_event();
	
private:
	BC_Pixmap *images[9];
};

#endif
