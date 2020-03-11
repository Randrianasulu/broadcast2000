#include <string.h>
#include "arender.h"
#include "arenderthread.h"
#include "atrack.h"
#include "audiodevice.h"
#include "autos.h"
#include "auto.h"
#include "cache.h"
#include "commonrenderthread.h"
#include "console.h"
#include "edit.h"
#include "levelwindow.h"
#include "mainwindow.h"
#include "modules.h"
#include "playabletracks.h"
#include "renderengine.h"


ARender::ARender(MainWindow *mwindow, RenderEngine *renderengine)
: CommonRender(mwindow, renderengine)
{
	amodule_render_fragment = 0;
	audio_out_position = 0;
	audio_out = 0;
	output_length = 0;
	input_length = 0;
}

ARender::~ARender()
{
}

int ARender::init_device_buffers()
{
// allocate output buffers if there is an audio device
	if(renderengine->audio)
	{
		audio_out = new float*[total_out_buffers];
		for(int i = 0; i < total_out_buffers; i++)
		{
			audio_out[i] = new float[output_length]; 
//printf("ARender::init_device_buffers 5 %x %d\n", audio_out[i], output_length);
		}
		audio_out_position = 0;
	}
return 0;
}


int ARender::get_datatype()
{
	return TRACK_AUDIO;
return 0;
}

int ARender::init_meters()
{
// prepare level meters and peaks here
// not providing enough peaks results in peaks that are ahead of the sound
	meter_render_fragment = amodule_render_fragment;
	
	while(meter_render_fragment > mwindow->sample_rate / 10) meter_render_fragment /= 2;

	total_peaks = 2 * input_length / meter_render_fragment + 256000 / meter_render_fragment;

	current_peak = 0;
	peak_samples = new long[total_peaks];
	for(int i = 0; i < total_peaks; i++)
	{
		peak_samples[i] = -1;
	}

	if(mwindow->gui)
	{
		mwindow->level_window->init_meter(total_peaks);
// prepare module meters
		mwindow->console->modules->init_meters(total_peaks);
	}
return 0;
}

int ARender::arm_playback(long current_position,
			long input_length, 
			long amodule_render_fragment, 
			long playback_buffer, 
			long output_length)
{
	total_out_buffers = renderengine->output_channels;
	this->current_position = current_position;
	this->input_length = input_length;
	this->amodule_render_fragment = amodule_render_fragment;
	this->playback_buffer = playback_buffer;
	this->output_length = output_length;
	absolute_position = 0;
	startup_lock.lock();

	source_length = renderengine->end_position - renderengine->start_position;

	if(renderengine->real_time_playback) set_realtime();

	if(renderengine->audio)
	{
		init_device_buffers();
		init_meters();
	}

// start the thread waiting on input
	thread = new ARenderThread(mwindow, this);
	thread->start_rendering(0);

// start reading input and sending to arenderthread
// only if there's an audio device
	if(renderengine->audio)	
	{
		synchronous = 1;
		start();
	}
return 0;
}

int ARender::restart_playback()
{
// Use for rebuilding the virtual console during playback.
// Send last buffer to old thread.
	send_reconfigure_buffer();
	thread->wait_for_completion();

// The old console automatically transfers existing plugins to the new console.
	ARenderThread *new_thread = new ARenderThread(mwindow, this);
	new_thread->start_rendering(1);
	new_thread->wait_for_startup();

// The old console deletes plugins that aren't being used anymore and transfers
// the new plugins.
	thread->stop_rendering(1);
	delete thread;

	thread = new_thread;
return 0;
}

int ARender::send_reconfigure_buffer()
{
	if(renderengine->audio)
	{
		thread->output_lock[thread->current_input_buffer]->lock();

		thread->input_len[thread->current_input_buffer] = 0;
		thread->input_position[thread->current_input_buffer] = 0;
		thread->last_playback[thread->current_input_buffer] = 0;
		thread->last_reconfigure[thread->current_input_buffer] = 1;

		thread->input_lock[thread->current_input_buffer]->unlock();
		thread->swap_input_buffer();
	}
return 0;
}

int ARender::stop_rendering()
{
	if(renderengine->audio)
	{
// delete output buffers
		for(int i = 0; i < total_out_buffers; i++)
		{
			delete audio_out[i];
		}
		delete [] audio_out;
	}
	else
	{
// normally done in run()
		thread->stop_rendering(0);
	}

	delete thread;

// stop meters
	total_peaks = 0;
	current_peak = 0;

	if(mwindow->gui && renderengine->audio) 
	{
		delete [] peak_samples;
		mwindow->level_window->stop_meter();
		mwindow->console->modules->stop_meters();
	}
	total_out_buffers = 0;
return 0;
}

int ARender::process_buffer(float **buffer_out, long input_len, long input_position, int last_buffer)
{
	this->audio_out = buffer_out;
	this->last_playback = last_buffer;
	long fragment_position = 0, fragment_len = input_len;
	int reconfigure = 0;
	current_position = input_position;
	audio_out_position = 0;

	while(fragment_position < input_len)
	{
		fragment_len = input_len;
		if(fragment_position + fragment_len > input_len)
			fragment_len = input_len - fragment_position;

		reconfigure = test_virtualnodes(input_position, fragment_len, get_datatype(), 0);
		if(reconfigure) restart_playback();

		process_buffer(fragment_len, input_position);

		fragment_position += fragment_len;
		input_position += fragment_len;
		current_position = input_position;
	}
return 0;
}

int ARender::process_buffer(long input_len, long input_position)
{
// wait for an input_buffer to become available
	if(renderengine->audio)
	{
		thread->output_lock[thread->current_input_buffer]->lock();
	}

	if(!thread->interrupt)
	{
		float **buffer_in = ((ARenderThread*)thread)->buffer_in_ptr[thread->current_input_buffer];
		PluginBuffer **shared_buffer_in = ((ARenderThread*)thread)->shared_buffer_in_ptr[thread->current_input_buffer];

// =================== read data from tracks into input buffers
		for(int i = 0; i < thread->total_tracks; i++)
		{
			((ATrack*)thread->playable_tracks->values[i])->render(shared_buffer_in[i], 
							0,
							input_len, 
							renderengine->reverse ? input_position - input_len : input_position);
// 			((ATrack*)thread->playable_tracks->values[i])->render(buffer_in[i], 
// 							input_len, 
// 							renderengine->reverse ? input_position - input_len : input_position);

// Reverse buffer here so plugins always render forward.
			if(renderengine->reverse) reverse_buffer(buffer_in[i], input_len);
		}

		mwindow->cache->age_audio();

		thread->input_len[thread->current_input_buffer] = input_len;
		thread->input_position[thread->current_input_buffer] = input_position;
		thread->last_playback[thread->current_input_buffer] = last_playback;
		thread->last_reconfigure[thread->current_input_buffer] = 0;
		thread->absolute_position[thread->current_input_buffer] = absolute_position;

		if(renderengine->audio)
		{
// make this buffer available to thread
			thread->input_lock[thread->current_input_buffer]->unlock();
		}
		else
		{
// process this buffer now
			((ARenderThread*)thread)->process_buffer(thread->current_input_buffer, input_len, input_position, absolute_position);
		}

// go on to the next buffer
		absolute_position += input_len;
		thread->swap_input_buffer();
	}
return 0;
}

int ARender::reverse_buffer(float *buffer, long len)
{
	long start, end;
	float temp;

	for(start = 0, end = len - 1; end > start; start++, end--)
	{
		temp = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = temp;
	}
return 0;
}

int ARender::get_next_peak(int current_peak)
{
	current_peak++;
	if(current_peak == total_peaks) current_peak = 0;
	return current_peak;
return 0;
}

long ARender::get_render_length(long current_render_length)
{
	return current_render_length;
}

int ARender::wait_device_completion()
{
// audio device should be entirely cleaned up by arenderthread
	renderengine->audio->wait_for_completion();
return 0;
}

int ARender::wait_for_startup()
{
//	if(asynchronous) thread->wait_for_startup();
	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

void ARender::run()
{
	long current_input_length;
	int reconfigure = 0;

// The thread can be deleted during the first fragment so finish the startup
// locking here.
	thread->wait_for_startup();
	startup_lock.unlock();

	while(!done && !interrupt && !last_playback)
	{
		current_input_length = input_length;
		get_boundaries(current_input_length);

		if(current_input_length)
		{
			reconfigure = test_virtualnodes(current_position, current_input_length, get_datatype(), renderengine->reverse);
			if(reconfigure) restart_playback();
		}

		process_buffer(current_input_length, current_position);

		advance_position(get_render_length(current_input_length));
		if(thread->interrupt) interrupt = 1;
	}

	if(renderengine->realtime) wait_device_completion();
	thread->stop_rendering(0);
	stop_rendering();
}

