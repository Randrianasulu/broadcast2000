#include "sharpenwindow.h"


SharpenThread::SharpenThread(SharpenMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

SharpenThread::~SharpenThread()
{
}
	
void SharpenThread::run()
{
	window = new SharpenWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






SharpenWindow::SharpenWindow(SharpenMain *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 90, 200, 90, 0, !client->show_initially)
{ this->client = client; }

SharpenWindow::~SharpenWindow()
{
	delete sharpen_slider;
}

int SharpenWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Sharpness (Automated)"));
	y += 20;
	add_tool(sharpen_slider = new SharpenSlider(client, &(client->sharpness), x, y));
	y += 30;
	add_tool(sharpen_interlace = new SharpenInterlace(client, x, y));
}

int SharpenWindow::close_event()
{
	hide_window();
	client->save_defaults();
	client->send_hide_gui();
}

SharpenSlider::SharpenSlider(SharpenMain *client, float *output, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, *output, 0, MAXSHARPNESS, DKGREY, BLACK, 1)
{
	this->client = client;
	this->output = output;
}
SharpenSlider::~SharpenSlider()
{
}
int SharpenSlider::handle_event()
{
	*output = get_value();
	client->send_configure_change();
}




SharpenInterlace::SharpenInterlace(SharpenMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->interlace, "Interlace")
{
	this->client = client;
}
SharpenInterlace::~SharpenInterlace()
{
}
int SharpenInterlace::handle_event()
{
	client->interlace = get_value();
	client->send_configure_change();
}

