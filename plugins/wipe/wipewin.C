#include "wipewin.h"


WipeThread::WipeThread(WipeMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

WipeThread::~WipeThread()
{
}

void WipeThread::run()
{
	window = new WipeWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






WipeWin::WipeWin(WipeMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 110, 150, 110, 0, !client->show_initially)
{ this->client = client; }

WipeWin::~WipeWin()
{
	delete left;
	delete right;
	delete reverse;
}

int WipeWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Direction"));
	y += 20;
	add_tool(left = new WipeDirectionLeft(this, client, x, y));
	y += 20;
	add_tool(right = new WipeDirectionRight(this, client, x, y));
	y += 35;
	add_tool(reverse = new WipeReverse(client, x, y));
return 0;
}

int WipeWin::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

WipeDirectionLeft::WipeDirectionLeft(WipeWin *win, WipeMain *client, int x, int y)
 : BC_Radial(x, y, 16, 16, !client->direction, "Left")
{
	this->client = client;
	this->win = win;
}

WipeDirectionLeft::~WipeDirectionLeft()
{
}

int WipeDirectionLeft::handle_event()
{
	client->direction = get_value() ? 0 : 1;
	win->right->update(get_value() ^ 1);
	client->send_configure_change();
return 0;
}




WipeDirectionRight::WipeDirectionRight(WipeWin *win, WipeMain *client, int x, int y)
 : BC_Radial(x, y, 16, 16, client->direction, "Right")
{
	this->client = client;
	this->win = win;
}
WipeDirectionRight::~WipeDirectionRight()
{
}
int WipeDirectionRight::handle_event()
{
	client->direction = get_value() ? 1 : 0;
	win->left->update(get_value() ^ 1);
	client->send_configure_change();
return 0;
}






WipeReverse::WipeReverse(WipeMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
WipeReverse::~WipeReverse()
{
}
int WipeReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
return 0;
}
