#include "deinterwindow.h"


DeInterlaceThread::DeInterlaceThread(DeInterlaceMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

DeInterlaceThread::~DeInterlaceThread()
{
}

void DeInterlaceThread::run()
{
	window = new DeInterlaceWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






DeInterlaceWindow::DeInterlaceWindow(DeInterlaceMain *client)
 : BC_Window("", MEGREY, client->gui_string, 200, 170, 200, 170, 0, !client->show_initially)
{ this->client = client; }

DeInterlaceWindow::~DeInterlaceWindow()
{
	delete odd_fields;
	delete even_fields;
	delete average_fields;
	delete swap_fields;
}

int DeInterlaceWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Select lines to keep"));
	y += 25;
	add_tool(odd_fields = new DeInterlaceOdd(client, this, client->odd_fields, x, y, "Odd lines"));
	y += 25;
	add_tool(even_fields = new DeInterlaceEven(client, this, client->even_fields, x, y, "Even lines"));
	y += 25;
	add_tool(average_fields = new DeInterlaceAvg(client, this, client->average_fields, x, y, "Average lines"));
	y += 25;
	add_tool(swap_fields = new DeInterlaceSwap(client, this, client->swap_fields, x, y, "Swap lines"));
	y += 25;
//	add_tool(smart_fields = new DeInterlaceSmart(client, this, client->smart_fields, x, y, "Reverse telecine"));
	return 0;
}

int DeInterlaceWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

int DeInterlaceWindow::set_values(int even, int odd, int avg, int swap, int smart)
{
	client->odd_fields = odd;
	client->even_fields = even;
	client->average_fields = avg;
	client->swap_fields = swap;
	client->smart_fields = smart;
	odd_fields->update(odd);
	even_fields->update(even);
	average_fields->update(avg);
	swap_fields->update(swap);
	smart_fields->update(smart);
	client->send_configure_change();
return 0;
}


DeInterlaceOdd::DeInterlaceOdd(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text)
 : BC_CheckBox(x, y, 16, 16, output, text)
{
	this->client = client;
	this->window = window;
}
DeInterlaceOdd::~DeInterlaceOdd()
{
}
int DeInterlaceOdd::handle_event()
{
	if(!get_value())
	{
		update(1);
	}
	window->set_values(0, 1, 0, 0, 0);
return 0;
}

DeInterlaceEven::DeInterlaceEven(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text)
 : BC_CheckBox(x, y, 16, 16, output, text)
{
	this->client = client;
	this->window = window;
}
DeInterlaceEven::~DeInterlaceEven()
{
}
int DeInterlaceEven::handle_event()
{
	if(!get_value())
	{
		update(1);
	}
	window->set_values(1, 0, 0, 0, 0);
	return 1;
}

DeInterlaceAvg::DeInterlaceAvg(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text)
 : BC_CheckBox(x, y, 16, 16, output, text)
{
	this->client = client;
	this->window = window;
}
DeInterlaceAvg::~DeInterlaceAvg()
{
}
int DeInterlaceAvg::handle_event()
{
	if(!get_value())
	{
		update(1);
	}
	window->set_values(0, 0, 1, 0, 0);
	return 1;
}

DeInterlaceSwap::DeInterlaceSwap(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text)
 : BC_CheckBox(x, y, 16, 16, output, text)
{
	this->client = client;
	this->window = window;
}
DeInterlaceSwap::~DeInterlaceSwap()
{
}
int DeInterlaceSwap::handle_event()
{
	if(!get_value())
	{
		update(1);
	}
	window->set_values(0, 0, 0, 1, 0);
	return 1;
}

DeInterlaceSmart::DeInterlaceSmart(DeInterlaceMain *client, DeInterlaceWindow *window, int output, int x, int y, char *text)
 : BC_CheckBox(x, y, 16, 16, output, text)
{
	this->client = client;
	this->window = window;
}
DeInterlaceSmart::~DeInterlaceSmart()
{
}
int DeInterlaceSmart::handle_event()
{
	if(!get_value())
	{
		update(1);
	}
	window->set_values(0, 0, 0, 0, 1);
	return 1;
}
