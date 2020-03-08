#include "dbxwindow.h"


DBXThread::DBXThread(DBXMain *client)
 : Thread()
{
	this->client = client;
	set_synchronous(1); // make thread wait for join
	gui_started.lock();
}

DBXThread::~DBXThread()
{
}
	
void DBXThread::run()
{
	window = new DBXWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






DBXWindow::DBXWindow(DBXMain *client)
 : BC_Window("", 
 	INFINITY, 
	INFINITY, 
	200, 
	120, 
	200, 
	120, 
	0, 
	0,
	!client->show_initially)
{ this->client = client; }

DBXWindow::~DBXWindow()
{
}

int DBXWindow::create_objects()
{
	int x = 110, y = 10;
	add_subwindow(new BC_Title(10, y + 5, "Gain:"));
	add_subwindow(gain = new DBXGain(client, x, y));
	y += 50;
	add_subwindow(new BC_Title(10, y + 5, "Window size:"));
	add_subwindow(window = new DBXWindowPot(client, x, y));
	return 0;
}

int DBXWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
	return 1;
}

DBXGain::DBXGain(DBXMain *plugin, int x, int y)
 : BC_FPot(x, 
		y, 
		plugin->gain, 
		INFINITYGAIN, 
		12)
{
	this->plugin = plugin;
}
int DBXGain::handle_event()
{
	plugin->gain = get_value();
	plugin->send_configure_change();
}



DBXWindowPot::DBXWindowPot(DBXMain *plugin, int x, int y)
 : BC_IPot(x, 
		y, 
		plugin->window, 
		8, 
		1024)
{
	this->plugin = plugin;
}
int DBXWindowPot::handle_event()
{
	plugin->window = get_value();
	plugin->send_configure_change();
}
