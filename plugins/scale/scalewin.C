#include "scalewin.h"


ScaleThread::ScaleThread(ScaleMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

ScaleThread::~ScaleThread()
{
}

void ScaleThread::run()
{
	window = new ScaleWin(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






ScaleWin::ScaleWin(ScaleMain *client)
 : BC_Window("", MEGREY, client->gui_string, 150, 150, 150, 150, 0, !client->show_initially)
{ this->client = client; }

ScaleWin::~ScaleWin()
{
	delete width;
	delete height;
	delete constrain;
}

int ScaleWin::create_objects()
{
	int x = 10, y = 10;
	char string[1024];

	add_tool(new BC_Title(x, y, "X Scale:"));
	y += 20;
	sprintf(string, "%.3f", client->scale_w);
	add_tool(width = new ScaleWidth(this, client, x, y, string));
	y += 30;
	add_tool(new BC_Title(x, y, "Y Scale:"));
	y += 20;
	sprintf(string, "%.3f", client->scale_h);
	add_tool(height = new ScaleHeight(this, client, x, y, string));
	y += 35;
	add_tool(constrain = new ScaleConstrain(client, x, y));
}

int ScaleWin::close_event()
{
	hide_window();
	client->cleanup_gui();
	client->send_hide_gui();
}

ScaleWidth::ScaleWidth(ScaleWin *win, ScaleMain *client, int x, int y, char *string)
 : BC_TextBox(x, y, 100, string)
{
	this->client = client;
	this->win = win;
}

ScaleWidth::~ScaleWidth()
{
}

int ScaleWidth::handle_event()
{
	client->scale_w = atof(get_text());
	if(client->scale_w <= 0) client->scale_w = 1;
	if(client->scale_w > 100) client->scale_w = 1;

	if(client->constrain)
	{
		win->height->update(get_text());
		client->scale_h = client->scale_w;
	}
	client->send_configure_change();
}




ScaleHeight::ScaleHeight(ScaleWin *win, ScaleMain *client, int x, int y, char *string)
 : BC_TextBox(x, y, 100, string)
{
	this->client = client;
	this->win = win;
}
ScaleHeight::~ScaleHeight()
{
}
int ScaleHeight::handle_event()
{
	client->scale_h = atof(get_text());
	if(client->scale_h <= 0) client->scale_h = 1;
	if(client->scale_h > 100) client->scale_h = 1;

	if(client->constrain)
	{
		win->width->update(get_text());
		client->scale_w = client->scale_h;
	}
	client->send_configure_change();
}






ScaleConstrain::ScaleConstrain(ScaleMain *client, int x, int y)
 : BC_CheckBox(x, y, 16, 16, client->constrain, "Constrain ratio")
{
	this->client = client;
}
ScaleConstrain::~ScaleConstrain()
{
}
int ScaleConstrain::handle_event()
{
	client->constrain = get_value();
	client->send_configure_change();
}
