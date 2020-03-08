#include "levelwindow.h"


LevelThread::LevelThread(LevelMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

LevelThread::~LevelThread()
{
}
	
void LevelThread::run()
{
	window = new LevelWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






LevelWindow::LevelWindow(LevelMain *client)
 : BC_Window("", MEGREY, client->gui_string, 200, 60, 200, 60, 0, !client->show_initially)
{ this->client = client; }

LevelWindow::~LevelWindow()
{
	delete slider;
}

int LevelWindow::create_objects()
{
	add_tool(slider = new LevelSlider(client));
	add_tool(new BC_Title(10, 40, "(Automated)"));
}

int LevelWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}

LevelSlider::LevelSlider(LevelMain *client)
 : BC_FSlider(5, 5, 190, 30, 200, client->level, INFINITYGAIN, MAXLEVEL, DKGREY, BLACK, 1)
{
	this->client = client;
}
LevelSlider::~LevelSlider()
{
}
int LevelSlider::handle_event()
{
	client->level = get_value();
	client->send_configure_change();
}
