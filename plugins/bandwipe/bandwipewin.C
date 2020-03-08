#include "bandwipewin.h"


BandWipeThread::BandWipeThread(BandWipeMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BandWipeThread::~BandWipeThread()
{
}

void BandWipeThread::run()
{
	window = new BandWipeWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BandWipeWin::BandWipeWin(BandWipeMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 100, 150, 100, 0, !client->show_initially)
{ this->client = client; }

BandWipeWin::~BandWipeWin()
{
	delete total;
	delete reverse;
}

int BandWipeWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Total bands"));
	y += 20;
	add_tool(total = new BandWipeTotal(client, x, y));
	y += 35;
	add_tool(reverse = new BandWipeReverse(client, x, y));
}

int BandWipeWin::close_event()
{
	hide_window();
	client->send_hide_gui();
}

BandWipeTotal::BandWipeTotal(BandWipeMain *client, int x, int y)
 : BC_TextBox(x, y, 100, client->total_bands)
{
	this->client = client;
}
BandWipeTotal::~BandWipeTotal()
{
}
int BandWipeTotal::handle_event()
{
	client->total_bands = atol(get_text());
	client->send_configure_change();
}

BandWipeReverse::BandWipeReverse(BandWipeMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
BandWipeReverse::~BandWipeReverse()
{
}
int BandWipeReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
}
