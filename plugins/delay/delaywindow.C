#include "delaywindow.h"


DelayThread::DelayThread(DelayMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

DelayThread::~DelayThread()
{
}
	
void DelayThread::run()
{
	window = new DelayWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






DelayWindow::DelayWindow(DelayMain *client)
 : BC_Window("", MEGREY, client->gui_string, 200, 50, 0, 0, 0, !client->show_initially)
{ this->client = client; }

DelayWindow::~DelayWindow()
{
	delete slider;
}

int DelayWindow::create_objects()
{
	add_tool(slider = new DelayPot(client));
	add_tool(new BC_Title(60, 10, "Samples"));
return 0;
}

int DelayWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

DelayPot::DelayPot(DelayMain *client)
 : BC_IPot(10, 5, 40, 40, client->duration, 0, 44100, LTGREY, MEGREY)
{
	this->client = client;
}
DelayPot::~DelayPot()
{
}
int DelayPot::handle_event()
{
	client->duration = get_value();
	client->send_configure_change();
return 0;
}
