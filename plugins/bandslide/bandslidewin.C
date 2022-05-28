#include "bandslidewin.h"


BandSlideThread::BandSlideThread(BandSlideMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BandSlideThread::~BandSlideThread()
{
}

void BandSlideThread::run()
{
	window = new BandSlideWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BandSlideWin::BandSlideWin(BandSlideMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 100, 150, 100, 0, !client->show_initially)
{ this->client = client; }

BandSlideWin::~BandSlideWin()
{
	delete total;
	delete reverse;
}

int BandSlideWin::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Total bands"));
	y += 20;
	add_tool(total = new BandSlideTotal(client, x, y));
	y += 35;
	add_tool(reverse = new BandSlideReverse(client, x, y));
return 0;
}

int BandSlideWin::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

BandSlideTotal::BandSlideTotal(BandSlideMain *client, int x, int y)
 : BC_TextBox(x, y, 100, client->total_bands)
{
	this->client = client;
}
BandSlideTotal::~BandSlideTotal()
{
}
int BandSlideTotal::handle_event()
{
	client->total_bands = atol(get_text());
	client->send_configure_change();
return 0;
}

BandSlideReverse::BandSlideReverse(BandSlideMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->reverse, "Reverse tracks")
{
	this->client = client;
}
BandSlideReverse::~BandSlideReverse()
{
}
int BandSlideReverse::handle_event()
{
	client->reverse = get_value();
	client->send_configure_change();
return 0;
}
