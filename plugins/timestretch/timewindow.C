#include "timewindow.h"

TimeWindow::TimeWindow()
 : BC_Window("", MEGREY, "Time Stretch", 400, 150, 400, 150)
{ }

TimeWindow::~TimeWindow()
{
	delete ok;
	delete cancel;
	delete percentage;
	delete window_size;
}

int TimeWindow::create_objects(TimeStretch *plugin)
{
	this->plugin = plugin;
	add_tool(new BC_Title(10, 10, "Enter the percentage of the original duration to write:"));
	add_tool(ok = new TimeWindowOK(this));
	add_tool(cancel = new TimeWindowCancel(this));
	
	char string[1024];
	sprintf(string, "%.3f", plugin->percentage);
	add_tool(percentage = new TimeWindowAmount(plugin, string));
	add_tool(new BC_Title(10, 60, "Window size:"));
	sprintf(string, "%ld", plugin->window_size);
	add_tool(window_size = new TimeWindowWindow(plugin, string));
}

TimeWindowOK::TimeWindowOK(TimeWindow *window)
 : BC_BigButton(10, 120, "OK")
{ this->window = window; }

int TimeWindowOK::handle_event() { window->set_done(0); }

int TimeWindowOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
}

TimeWindowCancel::TimeWindowCancel(TimeWindow *window)
 : BC_BigButton(200, 120, "Cancel")
{ this->window = window; }

int TimeWindowCancel::handle_event() { window->set_done(1); }

int TimeWindowCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
}

TimeWindowAmount::TimeWindowAmount(TimeStretch *plugin, char *string)
 : BC_TextBox(5, 30, 200, string)
{
	this->plugin = plugin;
}

TimeWindowAmount::~TimeWindowAmount()
{
}
	
int TimeWindowAmount::handle_event()
{
	plugin->percentage = atof(get_text());
}

TimeWindowWindow::TimeWindowWindow(TimeStretch *plugin, char *string)
 : BC_TextBox(5, 80, 200, string)
{
	this->plugin = plugin;
}

TimeWindowWindow::~TimeWindowWindow()
{
}
	
int TimeWindowWindow::handle_event()
{
	plugin->window_size = atol(get_text());
}
