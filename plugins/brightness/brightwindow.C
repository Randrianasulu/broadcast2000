#include "brightwindow.h"


BrightThread::BrightThread(BrightnessMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BrightThread::~BrightThread()
{
}
	
void BrightThread::run()
{
	window = new BrightWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BrightWindow::BrightWindow(BrightnessMain *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 120, 200, 120, 0, !client->show_initially)
{ this->client = client; }

BrightWindow::~BrightWindow()
{
	delete bright_slider;
	delete contrast_slider;
}

int BrightWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Brightness"));
	y += 20;
	add_tool(bright_slider = new BrightSlider(client, &(client->brightness), x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Contrast"));
	y += 20;
	add_tool(contrast_slider = new BrightSlider(client, &(client->contrast), x, y));
return 0;
}

int BrightWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

BrightSlider::BrightSlider(BrightnessMain *client, float *output, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, *output, -100, 100, DKGREY, BLACK, 1)
{
	this->client = client;
	this->output = output;
}
BrightSlider::~BrightSlider()
{
}
int BrightSlider::handle_event()
{
	*output = get_value();
	client->send_configure_change();
return 0;
}
