#ifndef PLAYBACKCURSOR_H
#define PLAYBACKCURSOR_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mainwindow.inc"
#include "mutex.h"
#include "thread.h"
#include "timer.h"

class PlaybackCursor : public Thread
{
public:
	PlaybackCursor(MainWindow *mwindow);
	~PlaybackCursor();


	int arm_playback(long position, int view_follows_playback, int reverse, int follow_loop);
	int start_playback();
	int stop_playback();

	int wait_for_startup();
	int update_cursor(long new_position);
	void run();

	int view_follows_playback;
	Mutex cursor_lock, complete, startup_lock;
	MainWindow *mwindow;
	int is_running;
	long last_position;
	int follow_loop;
	long current_offset;
	int reverse, double_speed;
	Timer timer;
};

#endif
