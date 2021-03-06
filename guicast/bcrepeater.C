#include "bcrepeater.h"
#include "bcwindow.h"

BC_Repeater::BC_Repeater(BC_WindowBase *top_level, long delay)
 : Thread()
{
	set_synchronous(1);
	repeating = 0;
	interrupted = 0;
	this->delay = delay;
	this->top_level = top_level;

	pause_lock.lock();
	startup_lock.lock();
	start();
	startup_lock.lock();
	startup_lock.unlock();
}

BC_Repeater::~BC_Repeater()
{
	interrupted = 1;
	pause_lock.unlock();
	repeat_lock.unlock();
	Thread::end();
	Thread::join();
}

int BC_Repeater::start_repeating()
{
	repeating++;
	if(repeating == 1)
	{
// Resume the loop
		pause_lock.unlock();
	}
	return 0;
}

int BC_Repeater::stop_repeating()
{
	repeating--;
// Pause the loop
	if(repeating == 0) pause_lock.lock();
	return 0;
}

void BC_Repeater::run()
{
	next_delay = delay;
	Thread::disable_cancel();
	startup_lock.unlock();

	while(!interrupted)
	{
		Thread::enable_cancel();
		timer.delay(next_delay);
		Thread::disable_cancel();
		timer.update();

// Test exit conditions
		if(interrupted) return;
		if(repeating <= 0) continue;

// Test for pause
		pause_lock.lock();
		pause_lock.unlock();

// Test exit conditions
		if(interrupted) return;
		if(repeating <= 0) continue;

// Wait for existing signal to be processed before sending a new one
		repeat_lock.lock();

// Test exit conditions
		if(interrupted)
		{
			repeat_lock.unlock();
			return;
		}
		if(repeating <= 0)
		{
			repeat_lock.unlock();
			continue;
		}

// Wait for window to become available.
		top_level->lock_window();

// Test exit conditions
		if(interrupted)
		{
			repeat_lock.unlock();
			top_level->unlock_window();
			return;
		}
		if(repeating <= 0)
		{
			repeat_lock.unlock();
			top_level->unlock_window();
			continue;
		}

// Stick event into queue
		top_level->arm_repeat(delay);
		top_level->unlock_window();
		next_delay = delay - timer.get_difference();
		if(next_delay <= 0) next_delay = 0;

// Test exit conditions
		if(interrupted) 
		{
			repeat_lock.unlock();
			return;
		}
		if(repeating <= 0)
		{
			repeat_lock.unlock();
			continue;
		}
	}
}
