#include "bcdragwindow.h"
#include "bcpixmap.h"

#include <unistd.h>

BC_DragWindow::BC_DragWindow(BC_WindowBase *parent_window, 
	BC_Pixmap *pixmap, 
	int icon_x, 
	int icon_y)
 : BC_Popup(parent_window, 
 	get_init_x(parent_window, icon_x), 
	get_init_y(parent_window, icon_y),
	pixmap->get_w(),
	pixmap->get_h(),
	-1,
	0,
	pixmap)
{
	init_x = get_x();
	init_y = get_y();
	end_x = INFINITY;
	end_y = INFINITY;
	icon_offset_x = init_x - parent_window->get_abs_cursor_x();
	icon_offset_y = init_y - parent_window->get_abs_cursor_y();
	do_animation = 1;
}

BC_DragWindow::~BC_DragWindow()
{
}

int BC_DragWindow::get_init_x(BC_WindowBase *parent_window, int icon_x)
{
	int output_x, temp = 0;
	Window tempwin;
	XTranslateCoordinates(parent_window->top_level->display, 
		parent_window->win, 
		parent_window->top_level->rootwin, 
		icon_x, 
		temp, 
		&output_x, 
		&temp, 
		&tempwin);
	return output_x;
}

int BC_DragWindow::get_init_y(BC_WindowBase *parent_window, int icon_y)
{
	int output_y, temp = 0;
	Window tempwin;
	XTranslateCoordinates(parent_window->top_level->display, 
		parent_window->win, 
		parent_window->top_level->rootwin, 
		temp, 
		icon_y, 
		&temp, 
		&output_y, 
		&tempwin);
	return output_y;
}

int BC_DragWindow::cursor_motion_event()
{
	reposition_window(get_abs_cursor_x() + icon_offset_x, 
		get_abs_cursor_y() + icon_offset_y, 
		get_w(), 
		get_h());
	return 1;
}

int BC_DragWindow::get_offset_x()
{
	return icon_offset_x;
}

int BC_DragWindow::get_offset_y()
{
	return icon_offset_y;
}

int BC_DragWindow::drag_failure_event()
{
	if(!do_animation) return 0;

	if(end_x == INFINITY)
	{
		end_x = get_x();
		end_y = get_y();
	}

	for(int i = 0; i < 10; i++)
	{
		int new_x = end_x + (init_x - end_x) * i / 10;
		int new_y = end_y + (init_y - end_y) * i / 10;

		reposition_window(new_x, 
			new_y, 
			get_w(), 
			get_h());
		flush();
		usleep(1000);
	}
	return 0;
}

void BC_DragWindow::set_animation(int value)
{
	this->do_animation = value;
}
