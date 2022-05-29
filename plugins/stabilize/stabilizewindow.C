#include "stabilizewindow.h"


StabilizeThread::StabilizeThread(StabilizeMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

StabilizeThread::~StabilizeThread()
{
}
	
void StabilizeThread::run()
{
	window = new StabilizeWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






StabilizeWindow::StabilizeWindow(StabilizeMain *client)
 : BC_Window("", MEGREY, client->gui_string, 190, 150, 190, 150, 0, !client->show_initially)
{ this->client = client; }

StabilizeWindow::~StabilizeWindow()
{
}

int StabilizeWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Image Stabilize"));
	y += 20;
	add_tool(new BC_Title(x, y + 10, "Search radius:"));
	add_tool(range = new StabilizeRange(client, x + 130, y));
	y += 40;
	add_tool(new BC_Title(x, y + 10, "Window size:"));
	add_tool(size = new StabilizeSize(client, x + 130, y));
	y += 40;
	add_tool(new BC_Title(x, y, "Acceleration:"));
	add_tool(new BC_Title(x, y + 20, "(Automated)"));
	add_tool(accel = new StabilizeAccel(client, x + 130, y));
return 0;
}

int StabilizeWindow::close_event()
{
	client->save_defaults();
	hide_window();
	client->send_hide_gui();
return 0;
}

StabilizeRange::StabilizeRange(StabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, (int)(client->range), 0, 256, DKGREY, BLACK)
{
	this->client = client;
}
StabilizeRange::~StabilizeRange()
{
}
int StabilizeRange::handle_event()
{
	client->range = get_value();
	client->send_configure_change();
return 0;
}

StabilizeSize::StabilizeSize(StabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, (int)(client->size), 0, 256, DKGREY, BLACK)
{
	this->client = client;
}
StabilizeSize::~StabilizeSize()
{
}
int StabilizeSize::handle_event()
{
	client->size = get_value();
	client->send_configure_change();
return 0;
}

StabilizeAccel::StabilizeAccel(StabilizeMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, (int)(client->size), 0, MAXACCEL, DKGREY, BLACK)
{
	this->client = client;
}
StabilizeAccel::~StabilizeAccel()
{
}
int StabilizeAccel::handle_event()
{
	client->accel = get_value();
	client->send_configure_change();
return 0;
}

