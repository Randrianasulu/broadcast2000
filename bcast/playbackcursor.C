#include <string.h>
#include "arender.h"
#include "console.h"
#include "levelwindow.h"
#include "mainwindow.h"
#include "modules.h"
#include "playbackcursor.h"
#include "playbackengine.h"
#include "renderengine.h"


PlaybackCursor::PlaybackCursor(MainWindow *mwindow)
 : Thread()
{ this->mwindow = mwindow; follow_loop = 0; }

PlaybackCursor::~PlaybackCursor()
{ }

int PlaybackCursor::arm_playback(long position, int view_follows_playback, int reverse, int follow_loop)
{
	this->view_follows_playback = view_follows_playback;
	is_running = 0;
	last_position = position;
	this->reverse = reverse;
	this->follow_loop = follow_loop;
	if(mwindow->gui) mwindow->show_playback_cursor(position);
return 0;
}

int PlaybackCursor::update_cursor(long new_position)
{
	if(mwindow->gui) mwindow->update_playback_cursor(new_position, view_follows_playback);
	last_position = new_position;
return 0;
}

int PlaybackCursor::start_playback()
{
	complete.lock();
	startup_lock.lock();
	is_running = 1;
	start();
return 0;
}

int PlaybackCursor::wait_for_startup()
{
	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

int PlaybackCursor::stop_playback()
{
	if(is_running)
	{
		is_running = 0;            // signal thread to stop
// don't end in the middle of a cursor update or a redraw for motion
		complete.lock();
		complete.unlock();
	}
	if(mwindow->gui) mwindow->hide_playback_cursor();
return 0;
}

void PlaybackCursor::run()
{
	long position, absolute_position;
	long last_peak = -1;
	long last_peak_number = 0; // for zeroing peaks
	long *peak_samples;
	long current_peak = 0, starting_peak;
	int total_peaks;
	int i, j, pass;
	int audio_on = mwindow->playback_engine->render_engine->audio_on;

	if(audio_on)
	{
		peak_samples = mwindow->playback_engine->render_engine->arender->peak_samples;
		total_peaks = mwindow->playback_engine->render_engine->arender->total_peaks;
	}
	else
	{
		peak_samples = 0;
		total_peaks = 0;
	}

	startup_lock.unlock();

	while(is_running)
	{
// select resets delay_duration
// delay between displays
		timer.delay(100);

		if(is_running)   // can be stopped during wait
		{
// Position for drawing cursor comes from rendered position
			position = mwindow->playback_engine->get_position(0);
			absolute_position = (long)(mwindow->playback_engine->absolute_position() * mwindow->playback_engine->speed);

// update cursor
			if(mwindow->gui) update_cursor(position);

// =================================== update meters

			if(audio_on)
			{
				starting_peak = current_peak;
				pass = 0;

				while(peak_samples[current_peak] < absolute_position && pass < 2)
				{
					current_peak++;
					if(current_peak == total_peaks) current_peak = 0;
					if(current_peak == starting_peak) pass++;
				}
				if(peak_samples[current_peak] > position)
				{
					current_peak--;
					if(current_peak < 0) current_peak = total_peaks - 1;
				}

				if(last_peak != peak_samples[current_peak])
				{
					if(mwindow->gui) mwindow->console->modules->update_meters(current_peak, last_peak_number, total_peaks);

					if(mwindow->gui) mwindow->level_window->update_meter(current_peak, last_peak_number, total_peaks);
					last_peak = peak_samples[current_peak];
					last_peak_number = current_peak;
				}
			}
		}
	}
	follow_loop = 0;

	complete.unlock();
}
