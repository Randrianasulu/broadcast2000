#include "swapwindow.h"




SwapThread::SwapThread(SwapMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

SwapThread::~SwapThread()
{
}

void SwapThread::run()
{
	window = new SwapWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}





SwapWindow::SwapWindow(SwapMain *client)
 : BC_Window("", MEGREY, client->gui_string, 250, 160, 250, 160, 0, !client->show_initially)
{
	this->client = client;
}

SwapWindow::~SwapWindow()
{
}

int SwapWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Swap channels"));
	y += 25;
	add_tool(new BC_Title(x + 160, y + 5, "-> Red"));
	add_tool(red = new SwapMenu(client, &(client->red), x, y));
	y += 25;
	add_tool(new BC_Title(x + 160, y + 5, "-> Green"));
	add_tool(green = new SwapMenu(client, &(client->green), x, y));
	y += 25;
	add_tool(new BC_Title(x + 160, y + 5, "-> Blue"));
	add_tool(blue = new SwapMenu(client, &(client->blue), x, y));
	y += 25;
	add_tool(new BC_Title(x + 160, y + 5, "-> Alpha"));
	add_tool(alpha = new SwapMenu(client, &(client->alpha), x, y));
return 0;
}

int SwapWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}





SwapMenu::SwapMenu(SwapMain *client, int *output, int x, int y)
 : BC_PopupMenu(x, y, 150, client->output_to_text(*output))
{
	this->client = client;
	this->output = output;
}
SwapMenu::~SwapMenu()
{
}
int SwapMenu::handle_event()
{
	client->send_configure_change();
return 0;
}

int SwapMenu::add_items()
{
	add_item(new SwapItem(this, client->output_to_text(RED_SRC)));
	add_item(new SwapItem(this, client->output_to_text(GREEN_SRC)));
	add_item(new SwapItem(this, client->output_to_text(BLUE_SRC)));
	add_item(new SwapItem(this, client->output_to_text(ALPHA_SRC)));
	add_item(new SwapItem(this, client->output_to_text(NO_SRC)));
	add_item(new SwapItem(this, client->output_to_text(MAX_SRC)));
return 0;
}




SwapItem::SwapItem(SwapMenu *menu, char *title)
 : BC_PopupItem(title)
{
	this->menu = menu;
}

SwapItem::~SwapItem()
{
}

int SwapItem::handle_event()
{
	menu->update(get_text());
	*(menu->output) = menu->client->text_to_output(get_text());
	menu->handle_event();
return 0;
}
