#include <string.h>
#include "bcbitmap.h"
#include "bccolors.h"
#include "bcresources.h"
#include "bcsubwindow.h"
#include "bctool.h"
#include "bcwindow.h"
#include "bcwindowbase.h"
#include "colormodels.h"

BC_ResizeCall::BC_ResizeCall(int w, int h)
{
	this->w = w;
	this->h = h;
}

// =============================== initialization

BC_WindowBase::BC_WindowBase(int x, int y, int w, int h, int color)
{
	this->x = x; this->y = y; this->w = w; this->h = h, this->color = color;
	subwindows = new BC_SubWindowList;
	tools = new BC_ToolList;
	border = 0;
	enabled = 1;
	top_level = 0;
	parent_window = 0;
}

BC_WindowBase::~BC_WindowBase()
{
// =================================== delete subwindows
	delete subwindows;     // delete owned subwindow base classes

// delete the tools
	delete tools;

// delete the X window of this subwindow
	if(top_level->win != 0) 
	{
		top_level->delete_window(win);
		XDestroyWindow(top_level->display, win);
		XFlush(top_level->display);
		win = 0;
	}
	
	if(top_level == this)
	{
// This window must not be waiting on events when closed.
		XCloseDisplay(top_level->display);
		win = 0;
	}

	for(int i = 0; i < resize_history.total; i++)
		delete resize_history.values[i];
}

int BC_WindowBase::destroy_window()
{
// destroy just the X window
	for(BC_SubWindowItem* subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next) subwindow->pointer->destroy_window();

// destroy it's own window
	top_level->delete_window(win);
	XDestroyWindow(top_level->display, win);
	XFlush(top_level->display);
	win = 0;
return 0;
}

BC_SubWindow* BC_WindowBase::add_subwindow(BC_SubWindow *subwindow) 
{
	if(!parent_window) parent_window = this;
	
// add pointer to the subwindow to the list
	subwindows->append(subwindow); 

// build the subwindow
	subwindow->create_objects_(top_level, this); 
}

int BC_WindowBase::delete_subwindow(BC_SubWindow* subwindow)
{
	if(subwindow)            // delete the object
	subwindows->remove(subwindow);
	
// user must delete derived class
return 0;
}

BC_Tool* BC_WindowBase::add_tool(BC_Tool *tool)
{
	if(!parent_window) parent_window = this;

// add the object
	tools->append(tool);
	
// build the tool
	tool->create_tool_objects(top_level, this);

	return tool;
}

int BC_WindowBase::delete_tool(BC_Tool *tool)
{
// delete the object
// user must delete derived class
	tools->remove(tool);
return 0;
}

int BC_WindowBase::add_border()
{
	add_border(top_level->get_resources()->bg_light1, 
		top_level->get_resources()->bg_color, 
		top_level->get_resources()->bg_color, 
		top_level->get_resources()->bg_shadow1, 
		top_level->get_resources()->bg_shadow2);
return 0;
}

int BC_WindowBase::add_border(int light, int medium, int dark)
{
	add_border(light, light, medium, dark, dark);
return 0;
}

int BC_WindowBase::add_border(int light1, int light2, int medium, int dark1, int dark2)
{
	if(!parent_window) parent_window = this;
	
	border = 1;    
	this->light1 = light1;    
	this->light2 = light2;    
	this->medium = medium;
	this->dark1 = dark1;
	this->dark2 = dark2;
	draw_border();
return 0;
}


int BC_WindowBase::resize_window(int x, int y, int w, int h)
{
	XMoveResizeWindow(top_level->display, win, x, y, w, h);
	this->x = x; this->y = y; this->w = w; this->h = h;
	if(border) draw_border();
// send tools and menubar the new size
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->resize_event_(w, h);
	}
return 0;
}

int BC_WindowBase::resize_window(int w, int h)
{
	if(top_level == this) resize_history.append(new BC_ResizeCall(w, h));
	XResizeWindow(top_level->display, win, w, h);
	this->w = w; this->h = h;
	if(border) draw_border();
// send tools and menubar the new size
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->resize_event_(w, h);
	}
return 0;
}

int BC_WindowBase::get_w() { return w; return 0;
}
int BC_WindowBase::get_h() { return h; return 0;
}
int BC_WindowBase::get_x() { return x; return 0;
}
int BC_WindowBase::get_y() { return y; return 0;
}
int BC_WindowBase::get_color() { return color; return 0;
}

// ================================= event handlers

int BC_WindowBase::repeat_event_dispatch(long repeat_id)
{
	int result = 0;
	
// all repeat event handlers must return either 1 or 0 to trap the repeat
//printf("BC_WindowBase::repeat_event_dispatch 1\n");
	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow && !result; subwindow = subwindow->next)
	{
		result = subwindow->pointer->repeat_event_dispatch(repeat_id);
	}
	
//printf("BC_WindowBase::repeat_event_dispatch 2\n");
	for(BC_ToolItem *tool = tools->first; tool && !result; tool = tool->next)
	{
		result = tool->pointer->repeat_event_dispatch(repeat_id);
	}
//printf("BC_WindowBase::repeat_event_dispatch 3\n");
	
	return result;
return 0;
}

int BC_WindowBase::expose_event_dispatch()
{
// if window flashes black and white, gc is inverse
// refresh this window's border
	if(top_level->event_win == win && border) draw_border();

// send to all subwindows
	for(BC_SubWindowItem *subwindow = subwindows->first; subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->expose_event_dispatch();
	}
			
// send to all tools in this subwindow
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->expose_event_dispatch();
	}
return 0;
}

int BC_WindowBase::button_press_dispatch()
{
	int result = 0;

	cursor_x = parent_window->cursor_x - x;
	cursor_y = parent_window->cursor_y - y;
// give to user
	result = button_press();

	for(BC_SubWindowItem *subwindow = subwindows->last; 
		subwindow && !result; subwindow = subwindow->previous)
	{
		result = subwindow->pointer->button_press_dispatch();
	}

// Some tools need an out of bounds button press to deactivate
//	if(cursor_x > 0 && cursor_x < w && cursor_y > 0 && cursor_y < h)
//	{
		for(BC_ToolItem *tool = tools->last; tool && !result; tool = tool->previous)
		{
			result = tool->pointer->button_press_dispatch();
		}
//	}
	return result;
return 0;
}

int BC_WindowBase::button_release_dispatch()
{
	cursor_x = parent_window->cursor_x - x;
	cursor_y = parent_window->cursor_y - y;
//printf("BC_WindowBase::button_release_dispatch cursor_x %d cursor_y %d\n", cursor_x, cursor_y);
// give to user
	button_release();

	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->button_release_dispatch();
	}

	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->button_release_dispatch();
	}
return 0;
}

int BC_WindowBase::motion_event_dispatch()
{
	int result;
	result = 0;
	cursor_x = parent_window->cursor_x - x;
	cursor_y = parent_window->cursor_y - y;

	for(BC_SubWindowItem *subwindow = subwindows->last; 
		subwindow && !result; subwindow = subwindow->previous)
	{
		result = subwindow->pointer->motion_event_dispatch();
	}

	for(BC_ToolItem *tool = tools->last; tool && !result; tool = tool->previous)
	{
		result = tool->pointer->motion_event_dispatch();
	}

	if(!result) result = cursor_motion();    // give to user
	return result;
return 0;
}

int BC_WindowBase::resize_event_dispatch()
{
// only give to user on top level
// give user size of parent window
//resize_event(parent_window->w, parent_window->h);

// resize other subwindows
	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->resize_event_dispatch();
	}
	
// send tools and menubar the size of this window
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->resize_event_(w, h);
	}
return 0;
}

int BC_WindowBase::keypress_event_dispatch()
{
	int result = 0;
// give keypress to active tool first
	if(top_level->active_tool) result = top_level->active_tool->keypress_event_();

// give to all subwindows
	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow && top_level->key_pressed && !result; subwindow = subwindow->next)
	{
		result = subwindow->pointer->keypress_event_dispatch(); 
	}

// give to all tools
	for(BC_ToolItem *tool = tools->first; 
		tool && top_level->key_pressed && !result; tool = tool->next)
	{
		if(tool->pointer->enabled) result = tool->pointer->keypress_event_();
	}
	return result;
return 0;
}

int BC_WindowBase::cursor_left_dispatch()
{
	cursor_x = parent_window->cursor_x - x;
	cursor_y = parent_window->cursor_y - y;

	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->cursor_left_dispatch();
	}

	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->cursor_left_dispatch();
	}
return 0;
}

int BC_WindowBase::unhighlight()
{
	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->unhighlight();
	}

	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->unhighlight();
	}
return 0;
}

// =========================== configuration

// flash all the subwindows
int BC_WindowBase::flash()
{
	if(border) draw_border();

// flash all the subwindows
	for(BC_SubWindowItem *subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next)
	{
		subwindow->pointer->flash();
	}

// flash all the tools
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->flash();
	}
return 0;
}





// ============================= control

int BC_WindowBase::enable_window()
{
	enabled = 1;
	for(BC_SubWindowItem* subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next) subwindow->pointer->enable_window();
		
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->enable();
	}
return 0;
}

int BC_WindowBase::disable_window()
{
	enabled = 0;
	for(BC_SubWindowItem* subwindow = subwindows->first; 
		subwindow; subwindow = subwindow->next) subwindow->pointer->disable_window();
		
	for(BC_ToolItem *tool = tools->first; tool; tool = tool->next)
	{
		tool->pointer->disable();
	}
return 0;
}

int BC_WindowBase::lock_window() 
{ 
	top_level->lock_window(); 
return 0;
}
int BC_WindowBase::unlock_window() 
{ 
	top_level->unlock_window(); 
return 0;
}

int BC_WindowBase::find_first_textbox(BC_Tool **tool)
{
	int result = 0;

// search this subwindow for tool
	for(BC_ToolItem *test_tool = tools->first; test_tool && !result; test_tool = test_tool->next)
	{
		if(test_tool->pointer->uses_text())
		{
			*tool = test_tool->pointer;
			result = 1;
		}
	}

// search subwindows for tool if not found
	for(BC_SubWindowItem* test_subwindow = subwindows->first; 
		test_subwindow && !result; test_subwindow = test_subwindow->next)
	{
		result = test_subwindow->pointer->find_first_textbox(tool);
	}
	
	return result;
return 0;
}


int BC_WindowBase::find_next_textbox(BC_Tool **tool, int *result)
{
// search this subwindow for a tool
	for(BC_ToolItem *test_tool = tools->first; test_tool && *result != 2;)
	{
		if(*result == 0)
		{
// searching for active text box 
			if(top_level->active_tool == test_tool->pointer && test_tool->pointer->uses_text())
			{
				(*result)++;
			}
		}
		else
		{
// searching for next text box in line
			if(test_tool->pointer->uses_text())
			{
				*tool = test_tool->pointer;
				(*result)++;
			}
		}
		if(*result != 2) test_tool = test_tool->next;
	}

// try the subwindows
	for(BC_SubWindowItem* test_subwindow = subwindows->first; 
		test_subwindow && *result != 2; test_subwindow = test_subwindow->next)
	{
		test_subwindow->pointer->find_next_textbox(tool, result);
	}
return 0;
}

// =================================== drawing

int BC_WindowBase::draw_border()
{
	int lx,ly,ux,uy;
	int h = this->h, w = this->w;

	h--; w--;

	lx = 1;  ly = 1;
	ux = w-1;  uy = h-1;
 
	set_color(light1);
	draw_line(0, 0, ux, 0);
	draw_line(0, 0, 0, uy);
	set_color(light2);
	draw_line(lx, ly, ux - 1, ly);
	draw_line(lx, ly, lx, uy - 1);

	set_color(dark1);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
	set_color(dark2);
	draw_line(w, 0, w, h);
	draw_line(0, h, w, h);
return 0;
}

int BC_WindowBase::set_color(int color)
{
	XSetForeground(top_level->display, top_level->gc, top_level->get_color(color)); 
return 0;
}

int BC_WindowBase::draw_line(int x1, int y1, int x2, int y2) 
{
	XDrawLine(top_level->display, win, top_level->gc, x1, y1, x2, y2);
return 0;
}

int BC_WindowBase::get_cursor_x()
{ return cursor_x; return 0;
}

int BC_WindowBase::get_cursor_y()
{ return cursor_y; return 0;
}

int BC_WindowBase::get_keypress()
{ return top_level->key_pressed; return 0;
}

int BC_WindowBase::get_key_pressed()
{ return top_level->key_pressed; return 0;
}

BC_Resources* BC_WindowBase::get_resources()
{ return top_level->resources; }


int BC_WindowBase::get_button_down()
{
	return top_level->button_down;
// top_level->button_pressed only valid for button press events
	//if(top_level->button_down) return top_level->button_pressed;
	//else return 0;
return 0;
}

int BC_WindowBase::get_buttonpress()
{
	return top_level->button_pressed;
return 0;
}

int BC_WindowBase::colormodel_available(int color_model)
{
	switch(color_model)
	{
		case BC_YUV420P:
			return top_level->get_resources()->use_yuv;
			break;
	}
	return 0;
return 0;
}
