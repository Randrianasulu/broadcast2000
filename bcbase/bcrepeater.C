#include <string.h>
#include "bcrepeater.h"
#include "bcwindow.h"

BC_Repeater::BC_Repeater(BC_Window *top_level, long repeat_id, long delay)
 : Thread()
{
	synchronous = 1;
	this->repeat_id = repeat_id;
	this->delay = delay;
	this->top_level = top_level;
	interrupted = 0;
	repeating = 0;
}

BC_Repeater::~BC_Repeater()
{
}

int BC_Repeater::start_repeating()
{
	interrupted = 0;
	startup_lock.lock();
	start();
	wait_for_startup();
return 0;
}

int BC_Repeater::stop_repeating()
{
	if(repeating)
	{
		repeating = 0;
		interrupted = 1;
// If this routine is called during an event, the repeat mutex is locked.
//		top_level->repeat_mutex.unlock();
//		end();
//		join();
//		top_level->repeat_mutex.lock();
	}
return 0;
}

int BC_Repeater::wait_for_startup()
{
	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

void BC_Repeater::run()
{
	int interrupted2;
	repeating = 1;
	next_delay = delay;

// The value of interrupt must only change at one point here
	interrupted2 = interrupted;
	while(!interrupted2)
	{
		if(startup_lock.trylock()) startup_lock.unlock();
		timer.delay(next_delay);
		timer.update();

// Wait for top_level to handle any previous repeat and unlock this mutex.
		repeat_mutex.lock();
		interrupted2 = interrupted;
// Stick an event in the queue.
		top_level->arm_repeat(repeat_id, interrupted2);
		next_delay = delay - timer.get_difference();
		if(next_delay <= 0) next_delay = 0;
	}
	repeating = 0;
}
