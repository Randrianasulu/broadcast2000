#include "barndoorwin.h"


BarnDoorThread::BarnDoorThread(BarnDoorMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BarnDoorThread::~BarnDoorThread()
{
}

void BarnDoorThread::run()
{
	window = new BarnDoorWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BarnDoorWin::BarnDoorWin(BarnDoorMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 50, 150, 50, 0, !client->show_initially)
{ this->client = client; }

BarnDoorWin::~BarnDoorWin()
{
	delete reverse;
}

int BarnDoorWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(reverse = new BarnDoorReverse(client, x, y));
}

int BarnDoorWin::close_event()
{
	hide_window();
	client->send_hide_gui();
}





BarnDoorReverse::BarnDoorReverse(BarnDoorMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
BarnDoorReverse::~BarnDoorReverse()
{
}
int BarnDoorReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
}
