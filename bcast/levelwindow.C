#include <string.h>
#include "defaults.h"
#include "levelwindow.h"
#include "levelwindowgui.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"


LevelWindow::LevelWindow(MainWindow *mwindow)
 : Thread()
{
	this->mwindow = mwindow;
	peak_history = 0;
	gui = 0;
}

LevelWindow::~LevelWindow()
{
	if(gui) 
	{
		gui->set_done(0);
		join();
	}
	if(gui) delete gui;
}

int LevelWindow::load_defaults(Defaults *defaults)
{
	meter_visible = defaults->get("METERVISIBLE", 1);
	horizontal = defaults->get("HORIZONTAL", 0);
	int temp = mwindow->output_channels * 30 + 5 + 55;
	
	w = defaults->get("METERW", horizontal ? 120 : temp);
	h = defaults->get("METERH", horizontal ? temp : 260);
return 0;
}

int LevelWindow::update_defaults(Defaults *defaults)
{
	defaults->update("METERVISIBLE", meter_visible);
	defaults->update("HORIZONTAL", horizontal);
	defaults->update("METERW", gui->get_w());
	defaults->update("METERH", gui->get_h());
return 0;
}

int LevelWindow::create_objects()
{
	Thread::synchronous = 1;
	this->channels = mwindow->output_channels;
	if(mwindow->gui) 
	{
		gui = new LevelWindowGUI(this, w, h);
		for(int i = 0; i < channels; i++)
		{
			gui->new_meter();
		}
		gui->create_objects();
	}
return 0;
}


int LevelWindow::change_channels(int new_channels)
{
	if(gui)
	{
		gui->lock_window();
		if(new_channels > channels)
		{
			for(int i = channels; i < new_channels; i++)
			{
				gui->new_meter();
			}
		}
		else
		if(new_channels < channels)
		{
			for(int i = new_channels; i < channels && gui->total_meters; i++)
			{
				gui->delete_meter();
			}
		}

		channels = new_channels;
		gui->unlock_window();
	}
	channels = new_channels; // for no gui
return 0;
}

int LevelWindow::change_format()
{
	if(gui)
	{
		gui->lock_window();
		while(gui->total_meters > 0) gui->delete_meter();
		for(int i = 0; i < channels; i++) gui->new_meter();

		gui->unlock_window();
	}
return 0;
}

int LevelWindow::show_window()
{
	meter_visible = 1;
	if(gui)
	{
		gui->show_window();
		mwindow->gui->mainmenu->set_show_levels(1);
	}
return 0;
}

int LevelWindow::hide_window()
{
	if(gui)
	{
		meter_visible = 0;
		gui->hide_window();
		mwindow->gui->mainmenu->set_show_levels(0);
	}
return 0;
}

void LevelWindow::run()
{
	if(gui) gui->run_window();
}

int LevelWindow::init_meter(int total_peaks)
{
	peak_history = new float*[channels];
	for(int i = 0; i < channels; i++)
	{
		peak_history[i] = new float[total_peaks];
	}
return 0;
}

int LevelWindow::stop_meter()
{
	for(int i = 0; i < channels; i++)
	{
		delete peak_history[i];
		if(gui) gui->meters[i]->reset();
	}
	
	delete [] peak_history;
	peak_history = 0;
return 0;
}

int LevelWindow::update(int peak_number, int channel, float value)
{
	peak_history[channel][peak_number] = value;
return 0;
}

int LevelWindow::update_meter(int peak_number, int last_peak, int total_peaks)
{
	int i, j;

	if(gui && !gui->get_hidden() && peak_history)
	{
// zero unused peaks
		for(j = last_peak;  j != peak_number; )
		{
			for(i = 0; i < channels; i++)
			{
				peak_history[i][j] = 0;
			}
			j++;
			if(j >= total_peaks) j = 0;
		}


		gui->lock_window();
		float peak;
//printf("	LevelWindow::update_meter %d %f\n", peak_number, peak_history[0][peak_number]);
		for(i = 0; i < channels; i++)
		{
			peak = peak_history[i][peak_number];
			gui->meters[i]->update(peak, peak > 1);
		}
		gui->unlock_window();
	}
return 0;
}

int LevelWindow::reset()
{
	if(gui)
	{
		for(int i = 0; i < channels; i++)
		{
			gui->meters[i]->reset();
		}
	}
return 0;
}
