#include <string.h>
#include "console.h"
#include "consolescroll.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "modules.h"

Console::Console(MainWindow *mwindow) : Thread()
{
	this->mwindow = mwindow;
	pixel_start = 0;
	button_down = 0;
	gui = 0;
	modules = 0;
	reconfigure_trigger = 0;
}

Console::~Console()
{
	if(gui) 
	{
		gui->set_done(0);
		join();
	}
	
	delete modules;
	if(gui) delete gui;
	gui = 0;
// only pointers are removed here
}

int Console::create_objects(int w, int h, int console_hidden, int vertical)
{
	synchronous = 1;
	this->vertical = vertical;

	if(mwindow->gui)	
	{
		if(vertical) 
		h = CONSOLEH;
		else 
		w = CONSOLEW;

		gui = new ConsoleWindow(mwindow, w, h, console_hidden);
		gui->create_objects();
	}

	modules = new Modules(mwindow, this);
return 0;
}

int Console::flip_vertical()
{
	if(gui)
	{
		int pixel = 0;
		modules->flip_vertical(pixel, pixel_start);
	}
return 0;
}

int Console::redo_pixels()
{
	if(gui)
	{
		int pixel = 0;
		modules->redo_pixels(pixel);
		gui->scroll->update();
	}
return 0;
}

void Console::run()
{
	if(gui) gui->run_window();
}

int Console::update_defaults(Defaults *defaults)
{
	if(gui)
	{
		defaults->update("CONSOLEW", gui->get_w());
		defaults->update("CONSOLEH", gui->get_h());
		defaults->update("CONSOLEVERTICAL", vertical);
	}
return 0;
}

int Console::delete_project()
{
	modules->delete_all();
return 0;
}

int Console::change_channels(int oldchannels, int newchannels)
{
	modules->change_channels(oldchannels, newchannels);
return 0;
}

int Console::pixelmovement(int distance)
{
	if(gui)
	{
		pixel_start += distance;
		modules->pixelmovement(-distance);
	}
return 0;
}

int Console::add_audio_track()
{
	modules->add_audio_track();
	if(gui) gui->scroll->update();
return 0;
}

int Console::add_video_track()
{
	modules->add_video_track();
	if(gui) gui->scroll->update();
return 0;
}

int Console::delete_track()
{
	modules->delete_track();
	if(gui) gui->scroll->update();
return 0;
}


int Console::start_reconfigure(int unlock_window)
{
	if(mwindow->is_playing_back)
	{
// call stop_playback here because calling from console screws up window locking
		if(unlock_window) gui->unlock_window();
		mwindow->start_reconfigure(0);
		//sleep(1);
	}
return 0;
}

int Console::stop_reconfigure(int unlock_window)
{
	if(mwindow->is_playing_back)
	{
		mwindow->stop_reconfigure(0);
		if(unlock_window) gui->lock_window();
	}
return 0;
}

int Console::toggles_selected(int on, int show, int mute)
{
	return modules->toggles_selected(on, show, mute);
return 0;
}

int Console::select_all_toggles(int on, int show, int mute)
{
	return modules->select_all_toggles(on, show, mute);
return 0;
}

int Console::deselect_all_toggles(int on, int show, int mute)
{
	return modules->deselect_all_toggles(on, show, mute);
return 0;
}








ConsoleWindow::ConsoleWindow(MainWindow *mwindow, int w, int h, int console_hidden)
 : BC_Window(mwindow->display, MEGREY, ICONNAME ": Console", w, h, 0, 0, 0, console_hidden)
{
	this->mwindow = mwindow;
	console = mwindow->console;
}

ConsoleWindow::~ConsoleWindow()
{
	delete scroll;
}

int ConsoleWindow::create_objects()
{
	scroll = new ConsoleMainScroll(this);
	scroll->create_objects(get_w(), get_h());
return 0;
}


int ConsoleWindow::resize_event(int w, int h)
{
	if(console->vertical && h < CONSOLEH - CONSOLEH / 3) { flip_vertical(w, h); }
	else
	if(!console->vertical && w < CONSOLEW - CONSOLEW / 3) { flip_vertical(w, h); }
	else
	{
		int need_resize = 0;

		if(console->vertical)
		{
			if(h != CONSOLEH) { h = CONSOLEH; need_resize = 1; }
		}
		else 
		{
			if(w != CONSOLEW) { w = CONSOLEW; need_resize = 1; }
		}
		
		if(need_resize)
		{
			resize_window(w, h);
		}
		scroll->resize_event(w, h);
	}
return 0;
}

int ConsoleWindow::flip_vertical(int w, int h)
{
	console->vertical ^= 1;

	if(console->vertical) 
	{ resize_window(w, CONSOLEH); }
	else 
	{ resize_window(CONSOLEW, h); }

	scroll->flip_vertical(this->get_w(), this->get_h());
	console->flip_vertical();
return 0;
}

int ConsoleWindow::close_event()
{
	hide_window();
	mwindow->gui->mainmenu->set_show_console(0);
return 0;
}

int ConsoleWindow::keypress_event()
{
// locks up for some reason
	if(get_keypress() == 'w')
	{
		mwindow->gui->mainmenu->set_show_console(0);
		hide_window();
		return 1;
	}
	return 0;
return 0;
}

int ConsoleWindow::button_release()
{
	if(console->button_down && console->reconfigure_trigger)
	{
		console->button_down = 0;
		console->reconfigure_trigger = 0;
// restart the playback
		console->start_reconfigure(1);
		console->stop_reconfigure(1);
		return 1;
	}
	console->button_down = 0;
	return 0;
return 0;
}
