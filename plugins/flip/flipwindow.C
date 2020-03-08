#include "flipwindow.h"


FlipThread::FlipThread(FlipMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

FlipThread::~FlipThread()
{
}
	
void FlipThread::run()
{
	window = new FlipWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






FlipWindow::FlipWindow(FlipMain *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 120, 200, 120, 0, !client->show_initially)
{ this->client = client; }

FlipWindow::~FlipWindow()
{
	delete flip_vertical;
	delete flip_horizontal;
}

int FlipWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Vertical"));
	y += 20;
	add_tool(flip_vertical = new FlipToggle(client, &(client->flip_vertical), x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Horizontal"));
	y += 20;
	add_tool(flip_horizontal = new FlipToggle(client, &(client->flip_horizontal), x, y));
}

int FlipWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}

FlipToggle::FlipToggle(FlipMain *client, int *output, int x, int y)
 : BC_CheckBox(x, y, 16, 16, *output)
{
	this->client = client;
	this->output = output;
}
FlipToggle::~FlipToggle()
{
}
int FlipToggle::handle_event()
{
	*output = get_value();
	client->send_configure_change();
}
