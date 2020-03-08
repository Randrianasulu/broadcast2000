#include <string.h>
#include "console.h"
#include "consolescroll.h"
#include "modules.h"

ConsoleMainScroll::ConsoleMainScroll(ConsoleWindow *gui)
{
	this->gui = gui;
	oldposition = 0;
	cxscroll = 0;
	cyscroll = 0;
}

ConsoleMainScroll::~ConsoleMainScroll()
{
}
	
int ConsoleMainScroll::create_objects(int w, int h)
{
	if(gui->console->vertical)
	gui->add_tool(cxscroll = new CXScroll(this, w, h));
	else
	gui->add_tool(cyscroll = new CYScroll(this, w, h));
return 0;
}

int ConsoleMainScroll::resize_event(int w, int h)
{
	if(cxscroll)
		cxscroll->set_size(0, h - 17, w, 17);	
	else
	if(cyscroll)
		cyscroll->set_size(w - 17, 0, 17, h);

	update();
return 0;
}

int ConsoleMainScroll::update()               // reflect new console view
{
	Console *console = gui->console;

	if(cxscroll)
		cxscroll->set_position(console->modules->total_pixels(), console->pixel_start, gui->get_w());
	else
	if(cyscroll)
		cyscroll->set_position(console->modules->total_pixels(), console->pixel_start, gui->get_h());

	oldposition = console->pixel_start;
return 0;
}

int ConsoleMainScroll::handle_event(long position)
{
	if(position != oldposition)
	{
		long distance = position - oldposition;
		oldposition = position;

		gui->console->pixel_start = position;
		gui->console->redo_pixels();
		//gui->console->pixelmovement(distance);
	}
return 0;
}

int ConsoleMainScroll::flip_vertical(int w, int h)
{
	if(cxscroll) delete cxscroll;
	if(cyscroll) delete cyscroll;
	
	cxscroll = 0;
	cyscroll = 0;
	
	if(gui->console->vertical)
	gui->add_tool(cxscroll = new CXScroll(this, w, h));
	else
	gui->add_tool(cyscroll = new CYScroll(this, w, h));

	update();
return 0;
}

CXScroll::CXScroll(ConsoleMainScroll *scroll, int w, int h)
 : BC_XScrollBar(0, h - 17, w, 17, 0, 0, 0)
{
	this->scroll = scroll;
}

CXScroll::~CXScroll()
{
}

int CXScroll::handle_event()
{
	scroll->handle_event(get_position());
return 0;
}

CYScroll::CYScroll(ConsoleMainScroll *scroll, int w, int h)
 : BC_YScrollBar(w - 17, 0, 17, h, 0, 0, 0)
{
	this->scroll = scroll;
}

CYScroll::~CYScroll()
{
}

int CYScroll::handle_event()
{
	scroll->handle_event(get_position());
return 0;
}
