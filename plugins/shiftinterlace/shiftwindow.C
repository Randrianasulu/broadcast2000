#include "shiftwindow.h"


ShiftThread::ShiftThread(ShiftInterlaceMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

ShiftThread::~ShiftThread()
{
}

void ShiftThread::run()
{
printf("ShiftThread::run 1\n");
	window = new ShiftWindow(client);
printf("ShiftThread::run 1\n");
	window->create_objects();
printf("ShiftThread::run 1\n");
	gui_started.unlock();
printf("ShiftThread::run 1\n");
	window->run_window();
printf("ShiftThread::run 1\n");
	delete window;
printf("ShiftThread::run 2\n");
}






ShiftWindow::ShiftWindow(ShiftInterlaceMain *client)
 : BC_Window("", MEGREY, client->gui_string, 220, 115, 220, 115, 0, !client->show_initially)
{ this->client = client; }

ShiftWindow::~ShiftWindow()
{
	delete odd_offset;
	delete even_offset;
}

int ShiftWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Even offset"));
	y += 20;
	add_tool(even_offset = new ShiftOffset(client, &client->even_offset, x, y));
	y += 35;
	add_tool(new BC_Title(x, y, "Odd offset"));
	add_tool(odd_offset = new ShiftOffset(client, &client->odd_offset, x, y));
return 0;
}

int ShiftWindow::close_event()
{
	client->save_defaults();
	hide_window();
	client->send_hide_gui();
return 0;
}

ShiftOffset::ShiftOffset(ShiftInterlaceMain *client, int *output, int x, int y)
 : BC_ISlider(x, y, 200, 30, 200, *output, -100, 100, 0, 1)
{
	this->client = client;
	this->output = output;
}

ShiftOffset::~ShiftOffset()
{
}

int ShiftOffset::handle_event()
{
	*output = get_value();
	client->send_configure_change();
	return 1;
}
