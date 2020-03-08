#include "blurwindow.h"


BlurThread::BlurThread(BlurMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BlurThread::~BlurThread()
{
}
	
void BlurThread::run()
{
	window = new BlurWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BlurWindow::BlurWindow(BlurMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 130, 150, 130, 0, !client->show_initially)
{ this->client = client; }

BlurWindow::~BlurWindow()
{
	delete vertical;
	delete horizontal;
	delete radius;
}

int BlurWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Blur"));
	y += 20;
	add_tool(horizontal = new BlurHorizontal(client, this, x, y));
	y += 30;
	add_tool(vertical = new BlurVertical(client, this, x, y));
	y += 25;
	add_tool(radius = new BlurRadius(client, x, y));
	x += 50;
	add_tool(new BC_Title(x, y, "Radius"));
	y += 20;
	add_tool(new BC_Title(x, y, "(Automated)"));
}

int BlurWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}

BlurRadius::BlurRadius(BlurMain *client, int x, int y)
 : BC_IPot(x, y, 35, 35, client->radius, 0, MAXRADIUS, DKGREY, BLACK)
{
	this->client = client;
}
BlurRadius::~BlurRadius()
{
}
int BlurRadius::handle_event()
{
	client->radius = get_value();
	client->send_configure_change();
}

BlurVertical::BlurVertical(BlurMain *client, BlurWindow *window, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->vertical, "Vertical")
{
	this->client = client;
	this->window = window;
}
BlurVertical::~BlurVertical()
{
}
int BlurVertical::handle_event()
{
	client->vertical = get_value();
	client->send_configure_change();
}

BlurHorizontal::BlurHorizontal(BlurMain *client, BlurWindow *window, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->horizontal, "Horizontal")
{
	this->client = client;
	this->window = window;
}
BlurHorizontal::~BlurHorizontal()
{
}
int BlurHorizontal::handle_event()
{
	client->horizontal = get_value();
	client->send_configure_change();
}
