#include "yuvwindow.h"


YUVThread::YUVThread(YUVMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

YUVThread::~YUVThread()
{
}
	
void YUVThread::run()
{
	window = new YUVWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






YUVWindow::YUVWindow(YUVMain *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 170, 200, 170, 0, !client->show_initially)
{ this->client = client; }

YUVWindow::~YUVWindow()
{
	delete y_slider;
	delete u_slider;
	delete v_slider;
	delete automation[0];
	delete automation[1];
	delete automation[2];
}

int YUVWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Y:"));
	add_tool(automation[0] = new AutomatedFn(client, this, x + 80, y, 0));
	y += 20;
	add_tool(y_slider = new YSlider(client, x, y));
	y += 35;
	add_tool(new BC_Title(x, y, "U:"));
	add_tool(automation[1] = new AutomatedFn(client, this, x + 80, y, 1));
	y += 20;
	add_tool(u_slider = new USlider(client, x, y));
	y += 35;
	add_tool(new BC_Title(x, y, "V:"));
	add_tool(automation[2] = new AutomatedFn(client, this, x + 80, y, 2));
	y += 20;
	add_tool(v_slider = new VSlider(client, x, y));
return 0;
}

int YUVWindow::close_event()
{
	client->save_defaults();
	hide_window();
	client->send_hide_gui();
return 0;
}

YSlider::YSlider(YUVMain *client, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, client->y, -MAXVALUE, MAXVALUE, DKGREY, BLACK, 1)
{
	this->client = client;
}
YSlider::~YSlider()
{
}
int YSlider::handle_event()
{
	client->y = get_value();
	client->send_configure_change();
return 0;
}

USlider::USlider(YUVMain *client, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, client->u, -MAXVALUE, MAXVALUE, DKGREY, BLACK, 1)
{
	this->client = client;
}
USlider::~USlider()
{
}
int USlider::handle_event()
{
	client->u = get_value();
	client->send_configure_change();
return 0;
}

VSlider::VSlider(YUVMain *client, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, client->v, -MAXVALUE, MAXVALUE, DKGREY, BLACK, 1)
{
	this->client = client;
}
VSlider::~VSlider()
{
}
int VSlider::handle_event()
{
	client->v = get_value();
	client->send_configure_change();
return 0;
}

AutomatedFn::AutomatedFn(YUVMain *client, YUVWindow *window, int x, int y, int number)
 : BC_CheckBox(x, y, 16, 16, client->automated_function == number, "Automate")
{
	this->client = client;
	this->window = window;
	this->number = number;
}

AutomatedFn::~AutomatedFn()
{
}

int AutomatedFn::handle_event()
{
	for(int i = 0; i < 3; i++)
	{
		if(i != number) window->automation[i]->update(0);
	}
	update(1);
	client->automated_function = number;
	client->send_configure_change();
return 0;
}

