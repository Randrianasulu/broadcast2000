#include <string.h>
#include "commonrender.h"
#include "commonrenderthread.h"
#include "mutex.h"
#include "playabletracks.h"
#include "pluginbuffer.h"
#include "renderengine.h"
#include "virtualnode.h"

CommonRenderThread::CommonRenderThread(MainWindow *mwindow, CommonRender *commonrender)
 : Thread()
{
	this->mwindow = mwindow;
	this->render_engine = commonrender->renderengine;
	this->commonrender = commonrender;
	total_virtual_modules = 0;
	interrupt = 0;
	done = 0;
	current_input_buffer = 0;
	current_thread_buffer = 0;
	startup_lock = new Mutex;
}

CommonRenderThread::~CommonRenderThread()
{
	delete startup_lock;
}

int CommonRenderThread::allocate_input_buffers()
{
	int i, j;

	if(render_engine->realtime && commonrender->asynchronous)
		total_buffers = MAX_BUFFERS;
	else
		total_buffers = 1;


// allocate the drive read buffers
	for(int buffer = 0; buffer < total_buffers; buffer++)
	{
		input_lock[buffer] = new Mutex;
		output_lock[buffer] = new Mutex;
		last_playback[buffer] = 0;
//		last_reconfigure[buffer] = 0;
		buffer_in[buffer] = allocate_input_buffer(buffer);

		input_lock[buffer]->lock();
	}
return 0;
}


int CommonRenderThread::sort_virtual_console()
{
// sort the console
	int done = 0, result = 0;
	long attempts = 0;
	int i;

	while(!done && attempts < 50)
	{
// sort iteratively until all the remaining plugins can be rendered
		done = 1;
		for(i = 0; i < total_virtual_modules; i++)
		{
			result = virtual_modules[i]->sort(&render_list);
			if(result) done = 0;
		}
		attempts++;
	}

// prevent short circuts
	if(attempts >= 50)
	{
		printf("CommonRenderThread::sort_virtual_console Failed to sort console.\n");
	}
return 0;
}

int CommonRenderThread::delete_virtual_console()
{
// delete the virtual modules
	for(int i = 0; i < total_virtual_modules; i++)
	{
		delete virtual_modules[i];
	}
	delete [] virtual_modules;

// delete sort order
	render_list.remove_all();
return 0;
}

int CommonRenderThread::delete_input_buffers()
{
// delete input buffers
	for(int buffer = 0; buffer < total_buffers; buffer++)
	{
		delete_input_buffer(buffer);
	}

	for(int i = 0; i < total_buffers; i++)
	{
		delete input_lock[i];
		delete output_lock[i];
	}

	delete playable_tracks;
	total_tracks = total_buffers = 0;
return 0;
}

int CommonRenderThread::start_rendering(int duplicate)
{
	this->interrupt = 0;
	init_rendering(duplicate);

	if(render_engine->realtime && commonrender->asynchronous)
	{
// don't start a thread unless writing to an audio device
		startup_lock->lock();
		Thread::synchronous = 1;   // prepare thread base class
		start();
	}
return 0;
}

int CommonRenderThread::wait_for_startup()
{
	if(render_engine->realtime && commonrender->asynchronous)
	{
		startup_lock->lock();
		startup_lock->unlock();
	}
return 0;
}

int CommonRenderThread::wait_for_completion()
{
	if(render_engine->realtime && commonrender->asynchronous)
	{
		join();
	}
return 0;
}

void CommonRenderThread::run()
{
	startup_lock->unlock();

	while(!done && !interrupt)
	{
// wait for a buffer to render through console
		input_lock[current_thread_buffer]->lock();

		if(!done && !interrupt && !last_reconfigure[current_thread_buffer])
		{
// render it if not last buffer
// send to output device or the previously set output buffer
			process_buffer(current_thread_buffer, input_len[current_thread_buffer], input_position[current_thread_buffer], absolute_position[current_thread_buffer]);

// test for exit conditions tied to the buffer
			if(last_playback[current_thread_buffer]) done = 1;

// free up buffer for reading from disk
			output_lock[current_thread_buffer]->unlock();

// get next buffer
			if(!done) swap_thread_buffer();
		}
		else
		if(last_reconfigure[current_thread_buffer])
			done = 1;

	}

	if(interrupt)
	{
//		commonrender->interrupt = 1;
		for(int i = 0; i < total_buffers; i++)
		{
			output_lock[i]->unlock();
		}
	}
	else
	if(!last_reconfigure[current_thread_buffer]){
		if(render_engine->realtime)
			send_last_output_buffer();
	}
}

int CommonRenderThread::swap_input_buffer()
{
	current_input_buffer++;
	if(current_input_buffer >= total_buffers) current_input_buffer = 0;
return 0;
}

int CommonRenderThread::swap_thread_buffer()
{
	current_thread_buffer++;
	if(current_thread_buffer >= total_buffers) current_thread_buffer = 0;
return 0;
}

