#ifndef BCREPEATER_H
#define BCREPEATER_H

#include "bcrepeater.inc"
#include "bcwindow.inc"
#include "mutex.h"
#include "thread.h"
#include "timer.h"

class BC_Repeater : public Thread
{
public:
	BC_Repeater(BC_Window *top_level, long repeat_id, long delay);
	~BC_Repeater();

	int start_repeating();
	int wait_for_startup();
	int stop_repeating();
	void run();

	long repeat_id;
	long delay;
	Mutex repeat_mutex;
	int interrupted;

private:
	Timer timer;
	int repeating;
	BC_Window *top_level;
	long next_delay;
	Mutex startup_lock;
};



#endif
