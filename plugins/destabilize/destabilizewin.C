#include "destabilizewin.h"


DestabilizeThread::DestabilizeThread(DestabilizeMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

DestabilizeThread::~DestabilizeThread()
{
}
	
void DestabilizeThread::run()
{
	window = new DestabilizeWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






DestabilizeWindow::DestabilizeWindow(DestabilizeMain *client)
 : BC_Window("", MEGREY, client->gui_string, 170, 110, 170, 110, 0, !client->show_initially)
{ this->client = client; }

DestabilizeWindow::~DestabilizeWindow()
{
}

int DestabilizeWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Image Destabilize"));
	y += 20;
	add_tool(new BC_Title(x, y + 10, "Range:"));
	add_tool(range = new DestabilizeRange(client, x + 100, y));
// 	y += 40;
// 	add_tool(new BC_Title(x, y + 10, "Acceleration:"));
// 	add_tool(accel = new DestabilizeAccel(client, x + 100, y));
	y += 40;
	add_tool(new BC_Title(x, y + 10, "Speed:"));
	add_tool(speed = new DestabilizeSpeed(client, x + 100, y));
}

int DestabilizeWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}

DestabilizeRange::DestabilizeRange(DestabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, client->range, 0, 100, DKGREY, BLACK)
{
	this->client = client;
}
DestabilizeRange::~DestabilizeRange()
{
}
int DestabilizeRange::handle_event()
{
	client->range = get_value();
	client->send_configure_change();
}

DestabilizeAccel::DestabilizeAccel(DestabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, (int)(client->accel), 0, 100, DKGREY, BLACK)
{
	this->client = client;
}
DestabilizeAccel::~DestabilizeAccel()
{
}
int DestabilizeAccel::handle_event()
{
	client->accel = get_value();
	client->send_configure_change();
}

DestabilizeSpeed::DestabilizeSpeed(DestabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, (int)client->speed, 0, 100, DKGREY, BLACK)
{
	this->client = client;
}
DestabilizeSpeed::~DestabilizeSpeed()
{
}
int DestabilizeSpeed::handle_event()
{
	client->speed = get_value();
	client->send_configure_change();
}
