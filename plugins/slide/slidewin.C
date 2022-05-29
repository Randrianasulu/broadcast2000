#include "slidewin.h"


SlideThread::SlideThread(SlideMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

SlideThread::~SlideThread()
{
}

void SlideThread::run()
{
	window = new SlideWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






SlideWin::SlideWin(SlideMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 110, 150, 110, 0, !client->show_initially)
{ this->client = client; }

SlideWin::~SlideWin()
{
	delete left;
	delete right;
	delete reverse;
}

int SlideWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Direction"));
	y += 20;
	add_tool(left = new SlideDirectionLeft(this, client, x, y));
	y += 20;
	add_tool(right = new SlideDirectionRight(this, client, x, y));
	y += 35;
	add_tool(reverse = new SlideReverse(client, x, y));
return 0;
}

int SlideWin::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

SlideDirectionLeft::SlideDirectionLeft(SlideWin *win, SlideMain *client, int x, int y)
 : BC_Radial(x, y, 16, 16, !client->direction, "Left")
{
	this->client = client;
	this->win = win;
}

SlideDirectionLeft::~SlideDirectionLeft()
{
}

int SlideDirectionLeft::handle_event()
{
	client->direction = get_value() ? 0 : 1;
	win->right->update(get_value() ^ 1);
	client->send_configure_change();
return 0;
}




SlideDirectionRight::SlideDirectionRight(SlideWin *win, SlideMain *client, int x, int y)
 : BC_Radial(x, y, 16, 16, client->direction, "Right")
{
	this->client = client;
	this->win = win;
}
SlideDirectionRight::~SlideDirectionRight()
{
}
int SlideDirectionRight::handle_event()
{
	client->direction = get_value() ? 1 : 0;
	win->left->update(get_value() ^ 1);
	client->send_configure_change();
return 0;
}






SlideReverse::SlideReverse(SlideMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
SlideReverse::~SlideReverse()
{
}
int SlideReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
return 0;
}
