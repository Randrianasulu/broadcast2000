#include "invertwindow.h"


InvertThread::InvertThread(InvertMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

InvertThread::~InvertThread()
{
}
	
void InvertThread::run()
{
	window = new InvertWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






InvertWindow::InvertWindow(InvertMain *client)
 : BC_Window("", MEGREY, client->gui_string, 100, 60, 100, 60, 0, !client->show_initially)
{ this->client = client; }

InvertWindow::~InvertWindow()
{
	delete invert;
}

int InvertWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Invert"));
	y += 20;
	add_tool(invert = new InvertToggle(client, &(client->invert), x, y));
return 0;
}

int InvertWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

InvertToggle::InvertToggle(InvertMain *client, int *output, int x, int y)
 : BC_CheckBox(x, y, 16, 16, *output)
{
	this->client = client;
	this->output = output;
}
InvertToggle::~InvertToggle()
{
}
int InvertToggle::handle_event()
{
	*output = get_value();
	client->send_configure_change();
return 0;
}
