#include "rgb601window.h"


RGB601Thread::RGB601Thread(RGB601Main *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

RGB601Thread::~RGB601Thread()
{
}
	
void RGB601Thread::run()
{
	window = new RGB601Window(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






RGB601Window::RGB601Window(RGB601Main *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 120, 200, 120, 0, !client->show_initially)
{ this->client = client; }

RGB601Window::~RGB601Window()
{
	delete rgb_to_601;
	delete _601_to_rgb;
}

int RGB601Window::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "RGB -> 601"));
	y += 20;
	add_tool(rgb_to_601 = new RGBto601Toggle(client, this, x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "601 -> RGB"));
	y += 20;
	add_tool(_601_to_rgb = new _601toRGBToggle(client, this, x, y));
}

int RGB601Window::close_event()
{
	client->cleanup_gui();
	hide_window();
	client->send_hide_gui();
}

RGBto601Toggle::RGBto601Toggle(RGB601Main *client, RGB601Window *window, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->rgb_to_601)
{
	this->client = client;
	this->window = window;
}
RGBto601Toggle::~RGBto601Toggle()
{
}
int RGBto601Toggle::handle_event()
{
	client->rgb_to_601 = get_value();
	client->_601_to_rgb = 0;
	window->_601_to_rgb->update(0);
	client->send_configure_change();
}

_601toRGBToggle::_601toRGBToggle(RGB601Main *client, RGB601Window *window, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->_601_to_rgb)
{
	this->client = client;
	this->window = window;
}
_601toRGBToggle::~_601toRGBToggle()
{
}
int _601toRGBToggle::handle_event()
{
	client->_601_to_rgb = get_value();
	client->rgb_to_601 = 0;
	window->rgb_to_601->update(0);
	client->send_configure_change();
}
