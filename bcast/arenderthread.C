#include <string.h>
#include "arender.h"
#include "arenderthread.h"
#include "atrack.h"
#include "audiodevice.h"
#include "console.h"
#include "edit.h"
#include "levelwindow.h"
#include "mainwindow.h"
#include "modules.h"
#include "playabletracks.h"
#include "pluginbuffer.h"
#include "renderengine.h"
#include "thread.h"
#include "tracks.h"
#include "virtualnode.h"
#include "virtualanode.h"


ARenderThread::ARenderThread(MainWindow *mwindow, ARender *arender)
 : CommonRenderThread(mwindow, arender)
{
	this->arender = arender;
}

ARenderThread::~ARenderThread()
{
}

int ARenderThread::init_rendering(int duplicate)
{
	playable_tracks = new PlayableTracks(mwindow, 
					arender->current_position, 
					render_engine->reverse, 
					TRACK_AUDIO);

	total_tracks = playable_tracks->total;

	allocate_input_buffers();
	build_virtual_console(duplicate, arender->current_position);
	sort_virtual_console();
	mwindow->console->modules->arender_init(arender->renderengine->real_time_playback, duplicate);
	mwindow->tracks->arender_init(arender->renderengine->real_time_playback, duplicate, arender->current_position);
	if(arender->renderengine->real_time_playback) set_realtime();
return 0;
}

int ARenderThread::build_virtual_console(int duplicate, long current_position)
{
// allocate the virtual modules and fork plugins
	total_virtual_modules = total_tracks;
	virtual_modules = new VirtualNode*[total_virtual_modules];
	PluginBuffer *buffer[MAX_BUFFERS];
	int i, j;

// build virtual console table
	for(i = 0; i < playable_tracks->total; i++)
	{
// create a double buffer of just one track
		for(j = 0; j < total_buffers; j++)
			buffer[j] = buffer_in[j][i];

		virtual_modules[i] = new VirtualANode(mwindow, 
			playable_tracks->values[i]->get_module_of(), 
			0, 
			0, 
			playable_tracks->values[i]->get_patch_of(), 
			playable_tracks->values[i],
			0,
			buffer,
			buffer,
			arender->input_length,
			arender->amodule_render_fragment,
			total_buffers, 
			1,
			1, 
			1,
			1,
			arender->total_out_buffers);

		virtual_modules[i]->expand(duplicate, current_position);
	}
return 0;
}

PluginBuffer** ARenderThread::allocate_input_buffer(int double_buffer)
{
	PluginBuffer **new_buffer;
	new_buffer = new PluginBuffer*[total_tracks];
	buffer_in_ptr[double_buffer] = new float*[total_tracks];
	shared_buffer_in_ptr[double_buffer] = new PluginBuffer*[total_tracks];
	for(int i = 0; i < total_tracks; i++)
	{
		new_buffer[i] = new PluginBuffer(arender->input_length, sizeof(float));
		buffer_in_ptr[double_buffer][i] = (float*)new_buffer[i]->get_data();
		shared_buffer_in_ptr[double_buffer][i] = new_buffer[i];
	}

	return new_buffer;
}

int ARenderThread::delete_input_buffer(int buffer)
{
	for(int i = 0; i < total_tracks; i++)
	{
		delete buffer_in[buffer][i];
	}
	delete [] buffer_in[buffer];
	delete [] buffer_in_ptr[buffer];
	delete [] shared_buffer_in_ptr[buffer];
return 0;
}


int ARenderThread::stop_rendering(int duplicate)
{
	wait_for_completion();

// stop plugins
	mwindow->console->modules->arender_stop(duplicate);
	mwindow->tracks->arender_stop(duplicate);

	delete_virtual_console();
	delete_input_buffers();
return 0;
}


int ARenderThread::process_buffer(int buffer,
							  long input_len,      // same for double and normal speed
							  long input_position, // start of input buffer in project
							  long absolute_position)  // Absolute position for peaks
{
	int i, j, k;
// length and lowest numbered sample of fragment in input buffer
	long fragment_len, fragment_position;	
// generic buffer
	float *current_buffer;
// starting sample of fragment in project
	long real_position;
// info for meters
	float min, max, peak;
	long meter_render_end;    // end of current meter fragment for getting levels
	long current_fragment_peak; // first meter peak in fragment

// process entire input buffer by filling one output buffer at a time
	for(fragment_position = 0; fragment_position < input_len && !interrupt; )
	{
// test for end of input buffer
		fragment_len = arender->amodule_render_fragment;
		if(fragment_position + fragment_len > input_len)
			fragment_len = input_len - fragment_position;

// test for end of output buffer
		if(arender->audio_out_position + fragment_len > arender->playback_buffer)
			fragment_len = arender->playback_buffer - arender->audio_out_position;

// clear output buffers
		for(i = 0; i < arender->total_out_buffers; i++)
		{
			current_buffer = &(arender->audio_out[i][arender->audio_out_position]);
			for(j = 0;  j < fragment_len; j++) current_buffer[j] = 0;
		}

// get the start of the fragment in the project
		real_position = render_engine->reverse ? input_position - fragment_position : input_position + fragment_position;

// render nodes in sorted list
		for(i = 0; i < render_list.total; i++)
		{
			((VirtualANode*)render_list.values[i])->render(arender->audio_out, 
					arender->audio_out_position, 
					buffer,
					fragment_position,
					fragment_len, 
					real_position, 
					arender->source_length,
					render_engine->reverse,
					arender);
		}

// get peaks and limit volume in the fragment
		current_fragment_peak = arender->current_peak;
		for(i = 0; i < arender->total_out_buffers; i++)
		{
			current_buffer = &(arender->audio_out[i][arender->audio_out_position]);
			arender->current_peak = current_fragment_peak;

			for(j = 0; j < fragment_len; )
			{
				if(render_engine->realtime)
					meter_render_end = j + arender->meter_render_fragment;
				else
					meter_render_end = fragment_len;
				
				if(meter_render_end > fragment_len) meter_render_end =  fragment_len;

				min = max = 0;

				for( ; j < meter_render_end; j++)
				{
					if(current_buffer[j] > max) max = current_buffer[j];
					else
					if(current_buffer[j] < min) min = current_buffer[j];

					if(current_buffer[j] > 1) current_buffer[j] = 1;
					else
					if(current_buffer[j] < -1) current_buffer[j] = -1;
				}

				if(fabs(max) > fabs(min))
					peak = fabs(max);
				else
					peak = fabs(min);

				if(render_engine->realtime && mwindow->gui) 
				{
					mwindow->level_window->update(arender->current_peak, i, peak);
//printf("ARenderThread::process_buffer %ld\n", absolute_position + fragment_position + j);
					arender->peak_samples[arender->current_peak] = absolute_position + fragment_position + j;
					arender->current_peak = arender->get_next_peak(arender->current_peak);
				}
			}
		}

// advance counters
		fragment_position += fragment_len;
		arender->audio_out_position += fragment_len;

// Output buffer is full.  Fix speed and send to device.
		if(render_engine->audio && !interrupt && 
			(arender->audio_out_position >= arender->playback_buffer ||
			last_playback[current_thread_buffer]))
		{
// speed parameters
			long real_output_len; // length compensated for speed
			float sample;       // output sample
			int k;

			for(i = 0; i < arender->total_out_buffers; i++)
			{
				current_buffer = arender->audio_out[i];
				register long in, out;
				long fragment_end;

// Time stretch the fragment to the real_output size
				if(render_engine->speed > 1)
				{
// Number of samples in real output buffer for each to sample rendered.
					int interpolate_len = (int)render_engine->speed;
					for(in = 0, out = 0; in < arender->audio_out_position; )
					{
						sample = 0;
						for(k = 0; k < interpolate_len; k++)
						{
							sample += current_buffer[in++];
						}
						sample /= render_engine->speed;
						current_buffer[out++] = sample;
					}
					real_output_len = out;
				}
				else
				if(render_engine->speed < 1)
				{
					int interpolate_len = (int)(1 / render_engine->speed); // number of samples to skip
					real_output_len = arender->audio_out_position * interpolate_len;

					for(in = arender->audio_out_position - 1, out = real_output_len - 1; in >= 0; )
					{
						for(k = 0; k < interpolate_len; k++)
						{
							current_buffer[out--] = current_buffer[in];
						}
						in--;
					}
				}
				else
					real_output_len = arender->audio_out_position;
			}

// write to device when full
			if(!render_engine->audio->get_interrupted())
			{
				render_engine->audio->write_buffer(arender->audio_out, 
					real_output_len, 
					arender->total_out_buffers);
			}

			if(render_engine->audio->get_interrupted()) interrupt = 1;
		}

// rotate the output position
		if(arender->audio_out_position >= arender->playback_buffer) 
		{
			arender->audio_out_position = 0;
		}
	}
return 0;
}

int ARenderThread::send_last_output_buffer()
{
	render_engine->audio->set_last_buffer();
return 0;
}
