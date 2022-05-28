#include "denoisewindow.h"


DenoiseThread::DenoiseThread(Denoise *plugin)
 : Thread()
{
	this->plugin = plugin;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

DenoiseThread::~DenoiseThread()
{
}
	
void DenoiseThread::run()
{
	window = new DenoiseWindow(plugin);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saved by plugin before this
	delete window;
}






DenoiseWindow::DenoiseWindow(Denoise *plugin)
 : BC_Window("", MEGREY, plugin->gui_string, 227, 100, 0, 0, 0, !plugin->show_initially)
{ this->plugin = plugin; }

DenoiseWindow::~DenoiseWindow()
{
}

int DenoiseWindow::create_objects()
{
	int x = 10, y = 10, i;
	add_tool(new BC_Title(x, y, "Window Size"));
	add_tool(windowsize = new DenoiseWindowSize(plugin, x + 100, y));
	y += 40;
	add_tool(new BC_Title(x, y, "Noise Level"));
	add_tool(noiselevel = new DenoiseNoiseLevel(plugin, x + 100, y));
//	y += 40;
//	add_tool(new BC_Title(x, y, "Output Level"));
//	add_tool(outputlevel = new DenoiseOutputLevel(plugin, this, db.todb(plugin->output_level), x + 100, y));
	add_tool(new DenoiseReset(plugin, x + 150, y));
	y += 20;
//	add_tool(new BC_Title(x, y, "Levels"));
//	add_tool(levels = new DenoiseLevels(plugin, x + 100, y));
//	y += 40;
//	add_tool(new BC_Title(x, y, "Quality"));
//	add_tool(iterations = new DenoiseIterations(plugin, x + 100, y));
return 0;
}

int DenoiseWindow::update_gui()
{
	noiselevel->update(plugin->noise_level);
//	outputlevel->update(db.todb(plugin->output_level));
	windowsize->update((int)plugin->window_size);
//	levels->update(plugin->levels);
//	iterations->update(plugin->iterations);
return 0;
}

int DenoiseWindow::close_event()
{
	hide_window();
	plugin->send_hide_gui();
return 0;
}








DenoiseNoiseLevel::DenoiseNoiseLevel(Denoise *plugin, int x, int y)
 : BC_IPot(x, y, 35, 35, plugin->noise_level, 0, 100, DKGREY, BLACK)
{
	this->plugin = plugin;
}

DenoiseNoiseLevel::~DenoiseNoiseLevel()
{
}

int DenoiseNoiseLevel::handle_event()
{
	plugin->noise_level = (float)get_value();
	plugin->send_configure_change();
return 0;
}



DenoiseOutputLevel::DenoiseOutputLevel(Denoise *plugin, DenoiseWindow *window, float value, int x, int y)
 : BC_FPot(x, y, 35, 35, value, INFINITYGAIN, 15, DKGREY, BLACK)
{
	this->plugin = plugin;
	this->window = window;
}

DenoiseOutputLevel::~DenoiseOutputLevel()
{
}

int DenoiseOutputLevel::handle_event()
{
	plugin->output_level = window->db.fromdb(get_value());
	plugin->send_configure_change();
return 0;
}



DenoiseWindowSize::DenoiseWindowSize(Denoise *plugin, int x, int y)
 : BC_TextBox(x, y, 100, (int)plugin->window_size)
{
	this->plugin = plugin;
}

DenoiseWindowSize::~DenoiseWindowSize()
{
}

int DenoiseWindowSize::handle_event()
{
	plugin->window_size = atol(get_text());
	plugin->send_configure_change();
return 0;
}


DenoiseLevels::DenoiseLevels(Denoise *plugin, int x, int y)
 : BC_IPot(x, y, 35, 35, plugin->levels, 1, 8, DKGREY, BLACK)
{
	this->plugin = plugin;
}

DenoiseLevels::~DenoiseLevels()
{
}

int DenoiseLevels::handle_event()
{
	plugin->levels = get_value();
	plugin->send_configure_change();
return 0;
}


DenoiseReset::DenoiseReset(Denoise *plugin, int x, int y)
 : BC_BigButton(x, y, "Reset")
{
	this->plugin = plugin;
}

DenoiseReset::~DenoiseReset()
{
}

int DenoiseReset::handle_event()
{
	plugin->reset();
	plugin->send_configure_change();
return 0;
}



DenoiseIterations::DenoiseIterations(Denoise *plugin, int x, int y)
 : BC_IPot(x, y, 35, 35, plugin->iterations, 1, 32, DKGREY, BLACK)
{
	this->plugin = plugin;
}

DenoiseIterations::~DenoiseIterations()
{
}

int DenoiseIterations::handle_event()
{
	plugin->iterations = get_value();
	plugin->send_configure_change();
return 0;
}


