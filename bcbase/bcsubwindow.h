#ifndef BCSUBWINDOW_H
#define BCSUBWINDOW_H

class BC_SubWindow;
class BC_SubWindows;
class BC_SubWindowItem;
class BC_SubWindowList;

#include "bcwindowbase.h"
#include "linklist.h"

class BC_SubWindow : public BC_WindowBase
{
public:
	BC_SubWindow() { };

// create the data for a subwindow to a window
	BC_SubWindow(int x, int y, int w, int h, int color = -1);

// destroy the subwindow
	virtual ~BC_SubWindow();


// user routines specific to subwindows

	int change_x(int distance);                   // change the x position of this window
	int change_y(int distance);                   // change the y position of this window

// ========== called by the parent window to actually create this window
	int create_objects_(BC_Window *top_level, BC_WindowBase *parent_window);

// create the subwindow window
	int create_window();
	BC_SubWindowItem *subwindow_item;      // list item to delete when this is deleted
};

class BC_SubWindowItem : public ListItem<BC_SubWindowItem>
{
public:
	BC_SubWindowItem(BC_SubWindow *pointer);
	~BC_SubWindowItem();
	
	BC_SubWindow *pointer;
};

class BC_SubWindowList : public List<BC_SubWindowItem>
{
public:
	BC_SubWindowList();
	~BC_SubWindowList();
	
	int append(BC_SubWindow *subwindow);
	int remove(BC_SubWindow *subwindow);
};

#endif
