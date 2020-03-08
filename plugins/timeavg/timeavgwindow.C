#include "timeavgwindow.h"


TimeAvgThread::TimeAvgThread(TimeAvgMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

TimeAvgThread::~TimeAvgThread()
{
}
	
void TimeAvgThread::run()
{
	window = new TimeAvgWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






TimeAvgWindow::TimeAvgWindow(TimeAvgMain *client)
 : BC_Window("", MEGREY, client->gui_string, 210, 80, 200, 80, 0, !client->show_initially)
{ this->client = client; }

TimeAvgWindow::~TimeAvgWindow()
{
}

int TimeAvgWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Frames to average"));
	y += 20;
	add_tool(total_frames = new TimeAvgSlider(client, x, y));
}

int TimeAvgWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}

TimeAvgSlider::TimeAvgSlider(TimeAvgMain *client, int x, int y)
 : BC_ISlider(x, y, 190, 30, 200, client->total_frames, 1, 30, DKGREY, BLACK, 1)
{
	this->client = client;
}
TimeAvgSlider::~TimeAvgSlider()
{
}
int TimeAvgSlider::handle_event()
{
	int result = get_value();
	if(result < 1 || result > 30) result = 1;
	client->total_frames = result;
	client->send_configure_change();
}
