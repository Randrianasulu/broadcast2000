#include <string.h>
#include "assets.h"
#include "cache.h"
#include "commonrenderthread.h"
#include "datatype.h"
#include "edits.h"
#include "mainwindow.h"
#include "playbackengine.h"
#include "playabletracks.h"
#include "pluginbuffer.h"
#include "preferences.h"
#include "preferencesthread.h"
#include "renderengine.h"
#include "strategies.inc"
#include "vrender.h"
#include "vedit.h"
#include "vframe.h"
#include "videoconfig.h"
#include "videodevice.h"
#include "vrenderthread.h"
#include "vtrack.h"


VRender::VRender(MainWindow *mwindow, RenderEngine *renderengine)
 : CommonRender(mwindow, renderengine)
{
	input_length = 0;
	vmodule_render_fragment = 0;
	playback_buffer = 0;
	absolute_frame = 0;
	asynchronous = 0;     // render 1 frame at a time
	framerate_counter = 0;
	video_out_buffer = 0;
	video_out = 0;
	render_strategy = -1;
}

VRender::~VRender()
{
}

int VRender::init_device_buffers()
{
// allocate output buffer if there is a video device
	if(renderengine->video)
	{
		video_out_buffer = 0;
		video_out = new VFrame*[playback_buffer];
		video_out[0] = 0;
		video_out_byte = 0;
		video_out_position = 0;
		render_strategy = -1;
	}
return 0;
}

int VRender::get_datatype()
{
	return TRACK_VIDEO;
return 0;
}

int VRender::arm_playback(long current_position,
			long input_length, 
			long module_render_fragment, 
			long playback_buffer, 
			int track_w,
			int track_h,
			int output_w,
			int output_h)
{
	this->current_position = tounits(current_position);
	if(renderengine->reverse) this->current_position--;    // Kludge
	this->input_length = input_length;
	this->vmodule_render_fragment = module_render_fragment;
	this->playback_buffer = playback_buffer;
	this->track_w = track_w;
	this->track_h = track_h;
	this->output_w = output_w;
	this->output_h = output_h;

	source_length = tounits(renderengine->end_position) - tounits(renderengine->start_position);
	if(renderengine->video)
	{
		init_device_buffers();
	}

// start the virtual console
	thread = new VRenderThread(mwindow, this);
	thread->start_rendering(0);
return 0;
}


int VRender::start_playback()
{
// start reading input and sending to vrenderthread
// use a thread only if there's a video device
	if(renderengine->realtime)
	{
		Thread::synchronous = 1;
		start();
	}
return 0;
}

int VRender::wait_for_startup()
{
return 0;
}

int VRender::restart_playback()
{
// Use for rebuilding the virtual console during playback.
// The old console automatically transfers existing plugins to the new console.
	VRenderThread *new_thread = new VRenderThread(mwindow, this);
	new_thread->start_rendering(1);

// The old console deletes plugins that aren't being used anymore and transfers
// the new plugins.
	thread->stop_rendering(1);
	delete thread;

	thread = new_thread;
return 0;
}

int VRender::stop_rendering()
{
// Done playing back realtime
	if(renderengine->video)
	{
		if(video_out)
		{
			if(video_out[0]) delete video_out[0];
			delete video_out;
		}
		if(video_out_buffer) delete video_out_buffer;
	}
	else
// Done rendering
	{
// normally done in run() but no thread was started
		thread->stop_rendering(0);
	}

	delete thread;
return 0;
}


int VRender::process_buffer(VFrame **buffer_out, long input_len, long input_position, int last_buffer)
{
// process buffer for non realtime
	int i, j;
	this->video_out = buffer_out;
	this->last_playback = last_buffer;
	long fragment_position = 0, fragment_len = input_len;
	int reconfigure = 0;

	current_position = input_position;
	video_out_position = 0;
// Don't delete the video_out buffer
	render_strategy = VRENDER_VPIXEL;

	while(fragment_position < input_len)
	{
		fragment_len = input_len;
		if(fragment_position + fragment_len > input_len)
			fragment_len = input_len - fragment_position;

// test for automation configuration and shorten the fragment len if necessary
		reconfigure = test_virtualnodes(input_position, fragment_len, get_datatype(), 0);
		if(reconfigure) restart_playback();

		process_buffer(fragment_len, input_position);

		fragment_position += fragment_len;
		input_position += fragment_len;
		current_position = input_position;
	}
	return 0;
return 0;
}

int VRender::process_buffer(long input_len, long input_position)
{
	long i, j;
	Edit *playable_edit;
	int new_render_strategy;

// Determine the rendering strategy for this frame.
	new_render_strategy = get_render_strategy(playable_edit, input_position);

// Reconfigure output frame
	if(new_render_strategy != render_strategy)
	{
		render_strategy = new_render_strategy;
		if(video_out[0]) delete video_out[0];
		if(video_out_buffer) delete video_out_buffer;
		video_out[0] = 0;
		video_out_buffer = 0;

		switch(render_strategy)
		{
			case VRENDER_MJPG:
				video_out[0] = new VFrame;
				break;

			case VRENDER_YUV420:
			case VRENDER_RGB565:
			case VRENDER_BGR8880:
// Get shared memory from videodevice.
// Create frame with shared memory.
				
				break;
			
			case VRENDER_RGB888:
				video_out_buffer = new PluginBuffer(output_w * output_h + 2, 3);
				video_out[0] = new VFrame((unsigned char*)video_out_buffer->get_data(), output_w, output_h, VFRAME_RGB888);
				break;

			case VRENDER_VPIXEL:
				video_out_buffer = new PluginBuffer(output_w * output_h + 1, sizeof(VPixel));
				video_out[0] = new VFrame((unsigned char*)video_out_buffer->get_data(), output_w, output_h, VFRAME_VPIXEL);
				break;
		}
	}

// Need to handle resize events for VDevice buffers here
	
// Render the frame
	switch(render_strategy)
	{
		case VRENDER_MJPG:
		{
			long compressed_size = ((VEdit*)playable_edit)->compressed_frame_size(input_position);
			video_out[0]->allocate_compressed_data(compressed_size);
			((VEdit*)playable_edit)->read_compressed_frame(video_out[0], input_position);
// Duplicated below
			mwindow->cache->age_video();
			((VRenderThread*)thread)->absolute_frame = absolute_frame;
			thread->input_len[0] = input_len;
			thread->input_position[0] = input_position;
		}
			break;

		case VRENDER_YUV420:
		case VRENDER_RGB565:
		case VRENDER_BGR8880:
			break;

		case VRENDER_RGB888:
			((VEdit*)playable_edit)->read_raw_frame(video_out[0], 
				video_out_buffer,
				video_out_byte, 
				input_position);
// Duplicated below
			mwindow->cache->age_video();
			((VRenderThread*)thread)->absolute_frame = absolute_frame;
			thread->input_len[0] = input_len;
			thread->input_position[0] = input_position;
			break;

		case VRENDER_VPIXEL:
// Read data from tracks into input buffers.
// Originally swapped the frame pointers in a temporary array 
// to get reverse rendering.
// Since 1 frame is now rendered at a time this is obsolete.
			VFrame ***buffer_in = ((VRenderThread*)thread)->buffer_in_ptr;
			PluginBuffer ***shared_buffer_in = ((VRenderThread*)thread)->shared_buffer_in_ptr;

			for(i = 0; i < thread->total_tracks; i++)
			{
				((VTrack*)thread->playable_tracks->values[i])->render(buffer_in[i], 
					shared_buffer_in[i][0],
					0, 
					input_len, 
					input_position, 
					1);
			}

// Duplicated above
			mwindow->cache->age_video();

			((VRenderThread*)thread)->absolute_frame = absolute_frame;
			thread->input_len[0] = input_len;
			thread->input_position[0] = input_position;

// process this buffer now in the thread
			thread->process_buffer(0, input_len, input_position, 0);
			break;
	}
	return 0;
return 0;
}

int VRender::get_render_strategy(Edit* &playable_edit, long input_position)
{
	Track *playable_track;
	ArrayList<int> *render_strategies;
	int result = VRENDER_VPIXEL;

	playable_edit = 0;

// Rendering in realtime.
	if(!renderengine->realtime) return result;

// Total number of playable tracks is 1
	if(thread->total_tracks != 1) return result;
	playable_track = thread->playable_tracks->values[0];

// Test conditions which are mutual between render.C and this.
	if(!playable_track->direct_copy_possible(input_position, input_position + 1))
		return result;

// Get render strategies supported by output device
	if(!(render_strategies = renderengine->video->get_render_strategies()))
		return result;

// Get the fastest render strategy for the edit under the current position
	playable_edit = ((VTrack*)playable_track)->edits->get_render_strategy(input_position, render_strategies, result);
	if(!playable_edit) return result;

// Asset and output device must have the same dimensions
	if(playable_edit->asset->width != output_w ||
		playable_edit->asset->height != output_h)
		return VRENDER_VPIXEL;

	return result;
return 0;
}


int VRender::flash_output()
{
// Can't do rendering for the LML.
	if(mwindow->preferences->vconfig->video_out_driver != PLAYBACK_LML ||
		video_out[0]->get_data())
		return renderengine->video->write_buffer(video_out[0]);
return 0;
}

void VRender::run()
{
	int reconfigure;


// Want to know how many samples rendering each frame takes.
// Then use this number to predict the next frame that should be rendered.
// Be suspicious of frames that render late so have a countdown
// before we start dropping.
	long current_sample, start_sample, end_sample; // Absolute counts.
	long next_frame;  // Actual position.
	long last_delay = 0;  // delay used before last frame
	long skip_countdown = VRENDER_THRESHOLD;    // frames remaining until drop
	long delay_countdown = VRENDER_THRESHOLD;  // Frames remaining until delay
	long current_input_total, current_input_length; // Count for advancing position.

	framerate_counter = 0;
	framerate_timer.update();

	while(!done && !renderengine->video->interrupt && !last_playback)
	{
// Perform the most time consuming part of frame decompression now.
// Want the condition before, since only 1 frame is rendered 
// and the number of frames skipped after this frame varies.
		current_input_length = input_length;    // 1 frame

		reconfigure = test_virtualnodes(current_position, current_input_length, get_datatype(), renderengine->reverse);
		if(reconfigure) restart_playback();

		process_buffer(input_length, current_position);

// Determine the delay until the frame needs to be shown.
		current_sample = (long)(renderengine->absolute_position() * renderengine->speed);
// latest sample at which the frame can be shown.
		end_sample = tosamples(absolute_frame, mwindow->sample_rate, mwindow->frame_rate);
// earliest sample by which the frame needs to be shown.
		start_sample = tosamples(absolute_frame - 1, mwindow->sample_rate, mwindow->frame_rate);

// Straight from XMovie
		if(end_sample < current_sample)
		{
// Frame rendered late.  Flash it now.
			flash_output();

			if(renderengine->every_frame)
			{
// User wants every frame.
				current_input_total = 1;
			}
			else
			if(skip_countdown > 0)
			{
// Maybe just a freak.
				current_input_total = 1;
				skip_countdown--;
			}
			else
			{
// Get the frames to skip.
				delay_countdown = VRENDER_THRESHOLD;
				current_input_total = 1 + (long)toframes(current_sample, mwindow->sample_rate, mwindow->frame_rate) - 
					(long)toframes(end_sample, mwindow->sample_rate, mwindow->frame_rate);
			}
		}
		else
		{
// Frame rendered early or just in time.
			current_input_total = 1;

			if(delay_countdown > 0)
			{
// Maybe just a freak
				delay_countdown--;
			}
			else
			{
				skip_countdown = VRENDER_THRESHOLD;
				if(start_sample > current_sample)
				{
// Came before the earliest sample so delay
					timer.delay((long)((float)(start_sample - current_sample) * 1000 / mwindow->sample_rate));
				}
				else
				{
// Came after the earliest sample so keep going
				}
			}

// Flash frame now.
			flash_output();
		}

		absolute_frame += current_input_total;

// advance position in project
		current_input_length = current_input_total;
		while(current_input_total && current_input_length && !last_playback)
		{
// set last_playback if necessary and decrease current_input_length
			get_boundaries(current_input_length);
// advance 1 frame
			advance_position(current_input_length);
			current_input_total -= current_input_length;
			current_input_length = current_input_total;
		}

// Calculate the framerate counter
		framerate_counter++;
		if(framerate_counter >= mwindow->frame_rate && 
			renderengine->realtime)
		{
			mwindow->preferences->actual_frame_rate = 
				(float)framerate_counter / 
				((float)framerate_timer.get_difference() / 1000);
			mwindow->preferences_thread->update_framerate();
			framerate_counter = 0;
			framerate_timer.update();
		}
	}

	if(renderengine->realtime) renderengine->video->stop_playback();
	thread->stop_rendering(0);
	stop_rendering();
}

long VRender::get_render_length(long current_render_length)
{
// not used
	return (long)(current_render_length * renderengine->get_framestep() + 
		renderengine->get_correction_factor(1));
}

long VRender::tounits(long position)
{
	return (long)toframes_round(position, mwindow->sample_rate, mwindow->frame_rate);
}

long VRender::fromunits(long position)
{
	return tosamples((float)position, mwindow->sample_rate, mwindow->frame_rate);
}
