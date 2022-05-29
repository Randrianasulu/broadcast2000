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
 : BC_Window("", MEGREY, client->gui_string, 200, 40, 200, 40, 0, !client->show_initially)
{ this->client = client; }

InvertWindow::~InvertWindow()
{
	delete toggle;
}

int InvertWindow::create_objects()
{
	add_tool(toggle = new InvertToggle(client));
	add_tool(new BC_Title(30, 10, "Invert"));
return 0;
}

int InvertWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

InvertToggle::InvertToggle(InvertMain *client)
 : BC_CheckBox(5, 5, 17, 17, client->invert)
{
	this->client = client;
}
InvertToggle::~InvertToggle()
{
}
int InvertToggle::handle_event()
{
	client->invert = get_value();
	client->send_configure_change();
return 0;
}
