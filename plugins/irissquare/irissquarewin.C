#include "irissquarewin.h"


IrisSquareThread::IrisSquareThread(IrisSquareMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

IrisSquareThread::~IrisSquareThread()
{
}

void IrisSquareThread::run()
{
	window = new IrisSquareWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






IrisSquareWin::IrisSquareWin(IrisSquareMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 50, 150, 50, 0, !client->show_initially)
{ this->client = client; }

IrisSquareWin::~IrisSquareWin()
{
	delete reverse;
}

int IrisSquareWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(reverse = new IrisSquareReverse(client, x, y));
}

int IrisSquareWin::close_event()
{
	hide_window();
	client->send_hide_gui();
}





IrisSquareReverse::IrisSquareReverse(IrisSquareMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
IrisSquareReverse::~IrisSquareReverse()
{
}
int IrisSquareReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
}
