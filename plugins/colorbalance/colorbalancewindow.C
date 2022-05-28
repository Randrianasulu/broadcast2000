#include "colorbalancewindow.h"


ColorBalanceThread::ColorBalanceThread(ColorBalanceMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

ColorBalanceThread::~ColorBalanceThread()
{
}

void ColorBalanceThread::run()
{
	window = new ColorBalanceWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






ColorBalanceWindow::ColorBalanceWindow(ColorBalanceMain *client)
 : BC_Window("", MEGREY, client->gui_string, 330, 160, 330, 160, 0, !client->show_initially)
{ this->client = client; }

ColorBalanceWindow::~ColorBalanceWindow()
{
}

int ColorBalanceWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Color Balance"));
	y += 25;
	add_tool(new BC_Title(x, y, "Cyan"));
	add_tool(cyan = new ColorBalanceSlider(client, &(client->cyan), x + 70, y));
	add_tool(new BC_Title(x + 270, y, "Red"));
	y += 25;
	add_tool(new BC_Title(x, y, "Magenta"));
	add_tool(magenta = new ColorBalanceSlider(client, &(client->magenta), x + 70, y));
	add_tool(new BC_Title(x + 270, y, "Green"));
	y += 25;
	add_tool(new BC_Title(x, y, "Yellow"));
	add_tool(yellow = new ColorBalanceSlider(client, &(client->yellow), x + 70, y));
	add_tool(new BC_Title(x + 270, y, "Blue"));
	y += 25;
	add_tool(preserve = new ColorBalancePreserve(client, x + 70, y));
	y += 25;
	add_tool(lock_params = new ColorBalanceLock(client, x + 70, y));
return 0;
}

int ColorBalanceWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

ColorBalanceSlider::ColorBalanceSlider(ColorBalanceMain *client, float *output, int x, int y)
 : BC_ISlider(x, y, 190, 15, 200, *output, -100, 100, DKGREY, BLACK, 0)
{
	this->client = client;
	this->output = output;
    old_value = *output;
}
ColorBalanceSlider::~ColorBalanceSlider()
{
}
int ColorBalanceSlider::handle_event()
{
	float difference = get_value() - *output;
	*output = get_value();
    client->synchronize_params(this, difference);
	client->send_configure_change();
return 0;
}

ColorBalancePreserve::ColorBalancePreserve(ColorBalanceMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->preserve, "Preserve luminosity")
{
	this->client = client;
}
ColorBalancePreserve::~ColorBalancePreserve()
{
}

int ColorBalancePreserve::handle_event()
{
	client->preserve = get_value();
	client->send_configure_change();
return 0;
}

ColorBalanceLock::ColorBalanceLock(ColorBalanceMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->lock_params, "Lock parameters")
{
	this->client = client;
}
ColorBalanceLock::~ColorBalanceLock()
{
}

int ColorBalanceLock::handle_event()
{
	client->lock_params = get_value();
	client->send_configure_change();
return 0;
}
