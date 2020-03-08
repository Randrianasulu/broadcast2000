#include <string.h>
#include "bcresources.h"
#include "bcsubwindow.h"
#include "bcwindow.h"

BC_SubWindow::BC_SubWindow(int x, int y, int w, int h, int color)
 : BC_WindowBase(x, y, w, h, color)
{  // base class stores up all the variables
	top_level = 0;
	parent_window= 0;
	subwindow_item = 0;   // not on a list yet
}

BC_SubWindow::~BC_SubWindow()
{     // base class deletes all the subwindows and tools
	if(subwindow_item)
	{
		subwindow_item->pointer = 0;   // stop an infinite loop
		delete subwindow_item;        // delete the list item that owns this tool
	}
}

int BC_SubWindow::create_objects_(BC_Window *top_level, BC_WindowBase *parent_window)
{
	if(!this->top_level)
	{
		this->parent_window = parent_window;
		this->top_level = top_level;
		if(color == -1)
			color = top_level->resources->get_bg_color();
		create_window();      // create the actual window
	}

	if(color == -1)
		color = top_level->resources->get_bg_color();

	create_objects();           // create user objects
return 0;
}


int BC_SubWindow::create_window()
{
// create the window
	unsigned long mask;
	XSetWindowAttributes attr;

	mask = CWEventMask | CWBackPixel | CWBorderPixel;
	attr.event_mask = ExposureMask;
	attr.background_pixel = top_level->get_color(color);
	attr.border_pixel = top_level->get_color(color);

// create the window in the parent's window with the top level display variables
	win = XCreateWindow(top_level->display, parent_window->win, x, y, w, h, 0, top_level->depth, InputOutput, top_level->vis, mask, &attr);
	XMapWindow(top_level->display, win);
	top_level->add_window(win);
return 0;
}

// ==================================== control

int BC_SubWindow::change_x(int distance)
{
	x += distance;
	if(x < -w || x > top_level->w)
	XMoveResizeWindow(top_level->display, win, -w, y, w, h);
	else
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
return 0;
}

int BC_SubWindow::change_y(int distance)
{
	y += distance;
	if(y < -h || x > top_level->h)
	XMoveResizeWindow(top_level->display, win, x, -h, w, h);
	else
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
return 0;
}




// ======================================== list


BC_SubWindowItem::BC_SubWindowItem(BC_SubWindow *pointer)
 : ListItem<BC_SubWindowItem>()
{
	this->pointer = pointer;
	pointer->subwindow_item = this;
}

BC_SubWindowItem::~BC_SubWindowItem()
{
	if(pointer)
	{
		pointer->subwindow_item = 0;      // stop an infinite loop from happening
		delete pointer;       // delete the subwindow object
	}
}

BC_SubWindowList::BC_SubWindowList()
 : List<BC_SubWindowItem>()
{
}

BC_SubWindowList::~BC_SubWindowList()
{
}

int BC_SubWindowList::append(BC_SubWindow *subwindow)
{
	List<BC_SubWindowItem>::append(new BC_SubWindowItem(subwindow));
return 0;
}

int BC_SubWindowList::remove(BC_SubWindow *subwindow)
{
	BC_SubWindowItem* current;
	for(current = first; current && current->pointer != subwindow; current = NEXT)
		;
	
	if(current) List<BC_SubWindowItem>::remove(current);
return 0;
}
