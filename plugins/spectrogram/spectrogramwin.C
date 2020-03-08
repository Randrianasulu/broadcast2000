#include "spectrogramwin.h"
#include "vframe.h"

SpectrogramThread::SpectrogramThread(Spectrogram *plugin)
 : Thread()
{
	this->plugin = plugin;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

SpectrogramThread::~SpectrogramThread()
{
}
	
void SpectrogramThread::run()
{
	window = new SpectrogramWindow(plugin);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saved by plugin before this
	delete window;
}






SpectrogramTrigger::SpectrogramTrigger(Spectrogram *plugin, SpectrogramWindow *window)
 : Thread()
{
	synchronous = 1; // make thread wait for join
	this->plugin = plugin;
	this->window = window;
	startup_lock.lock();
}
SpectrogramTrigger::~SpectrogramTrigger()
{
}

void SpectrogramTrigger::run()
{
	int result, done = 0;
	startup_lock.unlock();

	while(!done)
	{
		result = plugin->trigger->read_message();

		switch(result)
		{
			case -1:
				done = 1;
				break;

			case UPDATE_GUI:
			{
				window->lock_window();
				plugin->load_bitmap(window->bitmap);
				window->canvas->draw_bitmap(window->bitmap, 
						0, 0, 
						window->bitmap->get_w(), 
						window->bitmap->get_h(),
						0, 0, 
						window->bitmap->get_w(), 
						window->bitmap->get_h(), 0);
				window->canvas->flash();
				window->unlock_window();
			}
				break;
			
			case COMPLETED:
				done = 1;
				break;
		}
	}
}








SpectrogramWindow::SpectrogramWindow(Spectrogram *plugin)
 : BC_Window("", MEGREY, plugin->gui_string, plugin->w, plugin->h, 10, 10, 0, !plugin->show_initially)
{ 
	this->plugin = plugin; 
}

SpectrogramWindow::~SpectrogramWindow()
{
	delete bitmap;
}

int SpectrogramWindow::create_objects()
{
	int x = 10, y = 10, i;
	add_tool(new BC_Title(x, y, "Window Size:"));
	add_tool(windowsize = new SpectrogramWindowSize(plugin, x + 100, y));
	y += 30;
	add_tool(canvas = new SpectrogramCanvas(plugin, x, y));
	bitmap = new VFrame(0, plugin->get_canvas_w(), plugin->get_canvas_h());
	y += canvas->get_h() + 5;
	add_tool(freq_text = new BC_Title(x, plugin->h - 20, "--"));
}

int SpectrogramWindow::update_gui()
{
	lock_window();
	resize_window(plugin->w, plugin->h);
	canvas->set_size(canvas->get_x(), canvas->get_y(), plugin->get_canvas_w(), plugin->get_canvas_h());
	windowsize->update((int)plugin->window_size);
	delete bitmap;
	bitmap = new VFrame(0, plugin->get_canvas_w(), plugin->get_canvas_h());
	freq_text->resize_tool(10, plugin->h - 20);
	unlock_window();
}

int SpectrogramWindow::close_event()
{
	hide_window();
	plugin->cleanup_gui();
	plugin->send_hide_gui();
}

int SpectrogramWindow::resize_event(int w, int h)
{
	plugin->w = w;
	plugin->h = h;
	canvas->set_size(canvas->get_x(), canvas->get_y(), plugin->get_canvas_w(), plugin->get_canvas_h());
	delete bitmap;
	bitmap = new VFrame(0, plugin->get_canvas_w(), plugin->get_canvas_h());
	freq_text->resize_tool(10, h - 20);
	plugin->send_configure_change();
}

int SpectrogramWindow::update_freq_text(int cursor_x)
{
	char string[1024];
	int freq;

	freq = plugin->freq_table.tofreq((int)((float)cursor_x / canvas->get_w() * TOTALFREQS));

	sprintf(string, "%d Hz", freq);
	freq_text->update(string);
}








SpectrogramCanvas::SpectrogramCanvas(Spectrogram *plugin, int x, int y)
 : BC_Canvas(x, y, plugin->get_canvas_w(), plugin->get_canvas_h(), BLACK)
{
	this->plugin = plugin;
}

SpectrogramCanvas::~SpectrogramCanvas()
{
}

int SpectrogramCanvas::cursor_motion()
{
	if(get_cursor_x() > 0 && get_cursor_x() < get_w())
		plugin->thread->window->update_freq_text(get_cursor_x());
}




SpectrogramWindowSize::SpectrogramWindowSize(Spectrogram *plugin, int x, int y)
 : BC_TextBox(x, y, 100, (int)plugin->window_size)
{
	this->plugin = plugin;
}

SpectrogramWindowSize::~SpectrogramWindowSize()
{
}

int SpectrogramWindowSize::handle_event()
{
	if(atol(get_text()) <= 16)
		plugin->window_size = 16;
	else
		plugin->window_size = plugin->max_window_size(atol(get_text()));

	plugin->send_configure_change();
}
