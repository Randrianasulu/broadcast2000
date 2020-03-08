#include "convolvewindow.h"


ConvolveThread::ConvolveThread(Convolve *plugin)
 : Thread()
{
	this->plugin = plugin;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

ConvolveThread::~ConvolveThread()
{
}

void ConvolveThread::run()
{
	window = new ConvolveWindow(plugin);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saved by plugin before this
	delete window;
}






ConvolveWindow::ConvolveWindow(Convolve *plugin)
 : BC_Window("", MEGREY, plugin->gui_string, 227, 160, 227, 160, 0, !plugin->show_initially)
{ this->plugin = plugin; }

ConvolveWindow::~ConvolveWindow()
{
}

int ConvolveWindow::create_objects()
{
	int x = 10, y = 10, i;
	add_tool(new BC_Title(x, y, "Window Size:"));
	add_tool(windowsize = new ConvolveWindowSize(plugin, x + 100, y));
	y += 40;
	add_tool(new BC_Title(x, y, "Channel 1 level:"));
	y += 20;
	add_tool(automate_level[0] = new ConvolveAutomate(plugin, this, 0, x, y));
	add_tool(chan_level[0] = new ConvolveAmount(plugin, x + 150, y - 20, 0));
	y += 40;
	add_tool(new BC_Title(x, y, "Channel 2 level:"));
	y += 20;
	add_tool(automate_level[1] = new ConvolveAutomate(plugin, this, 1, x, y));
	add_tool(chan_level[1] = new ConvolveAmount(plugin, x + 150, y - 20, 1));
	return 0;
}

int ConvolveWindow::update_gui()
{
	chan_level[0]->update(plugin->chan_level[0]);
	chan_level[1]->update(plugin->chan_level[1]);
	automate_level[0]->update(plugin->automated_level[0]);
	automate_level[1]->update(plugin->automated_level[1]);
	windowsize->update((int)plugin->engine.window_size);
}

int ConvolveWindow::close_event()
{
	hide_window();
	plugin->send_hide_gui();
}








ConvolveAmount::ConvolveAmount(Convolve *plugin, int x, int y, int number)
 : BC_FPot(x, y, 40, 40, plugin->chan_level[number], INFINITYGAIN, 6, DKGREY, BLACK)
{
	this->plugin = plugin;
	this->number = number;
}

ConvolveAmount::~ConvolveAmount()
{
}

int ConvolveAmount::handle_event()
{
	plugin->chan_level[number] = get_value();
	plugin->send_configure_change();
}



ConvolveWindowSize::ConvolveWindowSize(Convolve *plugin, int x, int y)
 : BC_TextBox(x, y, 100, (int)plugin->engine.window_size)
{
	this->plugin = plugin;
}

ConvolveWindowSize::~ConvolveWindowSize()
{
}

int ConvolveWindowSize::handle_event()
{
	if(atol(get_text()) <= 16)
		plugin->engine.window_size = 16;
	else
		plugin->engine.window_size = atol(get_text());

	plugin->send_configure_change();
}




ConvolveAutomate::ConvolveAutomate(Convolve *plugin, 
	ConvolveWindow *window, 
	int number, 
	int x, 
	int y) : BC_CheckBox(x, y, 16, 16, plugin->automated_level[number], "Automate")
{
	this->plugin = plugin;
	this->window = window;
	this->number = number;
}

ConvolveAutomate::~ConvolveAutomate()
{
}

int ConvolveAutomate::handle_event()
{
	window->automate_level[!number]->update(!get_value());
	plugin->automated_level[number] = get_value();
	plugin->automated_level[!number] = !get_value();
	plugin->send_configure_change();
	return 0;
}
