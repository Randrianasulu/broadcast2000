#include "pitchwindow.h"


PitchThread::PitchThread(Pitch *plugin)
 : Thread()
{
	this->plugin = plugin;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

PitchThread::~PitchThread()
{
}

void PitchThread::run()
{
	window = new PitchWindow(plugin);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saved by plugin before this
	delete window;
}






PitchWindow::PitchWindow(Pitch *plugin)
 : BC_Window("", MEGREY, plugin->gui_string, 227, 105, 227, 105, 0, !plugin->show_initially)
{ this->plugin = plugin; }

PitchWindow::~PitchWindow()
{
}

int PitchWindow::create_objects()
{
	int x = 10, y = 10, i;
	add_tool(new BC_Title(x, y, "Window Size:"));
	add_tool(windowsize = new PitchWindowSize(plugin, x + 100, y));
	y += 40;
	add_tool(new BC_Title(x, y, "Frequency change (Automated):"));
	y += 20;
	add_tool(freq_offset = new PitchAmount(plugin, x + 150, y));
}

int PitchWindow::update_gui()
{
	freq_offset->update(plugin->freq_offset);
	windowsize->update((int)plugin->engine.window_size);
}

int PitchWindow::close_event()
{
	hide_window();
	plugin->send_hide_gui();
}








PitchAmount::PitchAmount(Pitch *plugin, int x, int y)
 : BC_IPot(x, y, 35, 35, plugin->freq_offset, MINOFFSET, 100 + MAXOFFSET, DKGREY, BLACK)
{
	this->plugin = plugin;
}

PitchAmount::~PitchAmount()
{
}

int PitchAmount::handle_event()
{
	plugin->freq_offset = get_value();
	plugin->send_configure_change();
}



PitchWindowSize::PitchWindowSize(Pitch *plugin, int x, int y)
 : BC_TextBox(x, y, 100, (int)plugin->engine.window_size)
{
	this->plugin = plugin;
}

PitchWindowSize::~PitchWindowSize()
{
}

int PitchWindowSize::handle_event()
{
	if(atol(get_text()) <= 16)
		plugin->engine.window_size = 16;
	else
		plugin->engine.window_size = atol(get_text());

	plugin->send_configure_change();
}
