#include <string.h>
#include "audiodevice.h"
#include "buttonbar.h"
#include "defaults.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "modules.h"
#include "patchbay.h"
#include "playbackcursor.h"
#include "playbackengine.h"
#include "preferences.h"
#include "renderengine.h"
#include "trackcanvas.h"
#include "videodevice.h"
#include "vrender.h"

PlaybackEngine::PlaybackEngine(MainWindow *mwindow)
 : Thread()
{
	this->mwindow = mwindow;
// reset parameters here that you want to be persistant between playbacks
	reset_parameters();
}

PlaybackEngine::~PlaybackEngine()
{
}


int PlaybackEngine::reset_parameters()
{
// called before every playback
	is_playing_back = 0;
	follow_loop = 0;
	speed = 1;
	reverse = 0;
	cursor = 0;
	last_position = 0;
	playback_start = playback_end = 0;
	infinite = 0;
	use_buttons = 0;
	audio = 0;
	video = 0;
	shared_audio = 0;
return 0;
}

int PlaybackEngine::init_parameters()
{
	is_playing_back = 1;
	update_button = 1;
	correction_factor = 0;

// correct playback buffer sizes
	input_length = 
		playback_buffer = 
		output_length = 
		audio_module_fragment = 
		mwindow->preferences->audio_module_fragment;

// get maximum actual buffer size written to device plus padding
	if(speed != 1) output_length = (long)(output_length / speed) + 16;   
	if(output_length < playback_buffer) output_length = playback_buffer;

// samples to read at a time is a multiple of the playback buffer greater than read_length
	while(input_length < mwindow->preferences->audio_read_length)
		input_length += playback_buffer;
return 0;
}

int PlaybackEngine::init_cursor()
{
// initialize playback cursor
	if(mwindow->gui)
	{
		cursor = new PlaybackCursor(mwindow);
		cursor->arm_playback(last_position, 
			mwindow->preferences->view_follows_playback, 
			reverse, 
			follow_loop);
	}
return 0;
}

int PlaybackEngine::init_audio_device()
{
// open audio device
	if(!shared_audio)
	{
		audio = new AudioDevice;
		audio->open_output(mwindow->preferences->aconfig, 
			mwindow->sample_rate, 
			output_length);
		if(mwindow->preferences->playback_software_timer) audio->set_software_positioning();
	}
	return 0;
return 0;
}

int PlaybackEngine::init_video_device()
{
// open audio device
	if(mwindow->preferences->actual_frame_rate < 0) 
		mwindow->preferences->actual_frame_rate = mwindow->frame_rate;
	video = new VideoDevice(mwindow);
	return video->open_output(mwindow->preferences->vconfig, 
		mwindow->frame_rate, 
		mwindow->output_w, 
		mwindow->output_h, 
		OUTPUT_RGB,
		0);
return 0;
}


int PlaybackEngine::set_range(long start,
					long end,
					long current,
					int reverse,
					float speed)
{
	this->playback_start = start;
	this->playback_end = end;
	this->last_position = current;
	this->reverse = reverse;
	this->speed = speed;
	if(speed == FRAME_SPEED)
	{
// Fix positions for frame advance
		long frame = (long)(((float)(last_position + 1) / mwindow->sample_rate) * mwindow->frame_rate);
		last_position = (long)(((float)frame / mwindow->frame_rate) * mwindow->sample_rate);

		if(reverse)
		{
			playback_end = last_position;
			playback_start = (long)(((float)(frame - 1) / mwindow->frame_rate) * mwindow->sample_rate);
		}
		else
		{
			playback_start = last_position;
			playback_end = (long)(((float)(frame + 1) / mwindow->frame_rate) * mwindow->sample_rate);
		}
	}
return 0;
}


int PlaybackEngine::arm_playback(int follow_loop, 
					int use_buttons, 
					int infinite, 
					AudioDevice *audio)
{
	this->follow_loop = follow_loop;
	this->infinite = infinite;
	this->use_buttons = use_buttons;
	this->audio = audio;
	is_playing_back = 1;   // Armed but no thread
	init_cursor();  // depends on values
return 0;
}

int PlaybackEngine::start_playback()
{
// Originally in arm_playback
	int need_audio = mwindow->patches->total_playable_atracks();
	int need_video = mwindow->patches->total_playable_vtracks();
	if(speed == FRAME_SPEED)
	{
		need_audio = 0;     // No audio during frame advance
	}
	if(!need_audio) audio = 0;    // Defeat a setting from arm_playback

// need to set these here since mwindow isn't created on time
	shared_audio = audio ? 1 : 0;

	init_parameters();
	if(need_audio) init_audio_device();
	if(need_video) init_video_device();

// initialize the render engine
	render_engine = new RenderEngine(mwindow, 
									this->audio, 
									video, 
									this); // resets on construction

	render_engine->arm_playback_common(playback_start,
						playback_end,
						last_position,
						reverse,
						speed,
						follow_loop,
						infinite,
						mwindow->preferences->real_time_playback);

	if(need_audio)
	{
		render_engine->arm_playback_audio(input_length, 
						audio_module_fragment, 
						playback_buffer, 
						output_length, 
						mwindow->output_channels);
	}

	if(need_video)
	{
		render_engine->arm_playback_video(mwindow->preferences->video_every_frame, 
						mwindow->preferences->video_read_length, 
						1, 
						mwindow->track_w,
						mwindow->track_h,
						mwindow->output_w,
						mwindow->output_h);
	}

	render_engine->wait_for_startup();


// Originally in start_playback
	is_playing_back = 2;
	complete.lock();
	timer.update();
	if(video) render_engine->start_video();	// start the video rendering
	if(audio) audio->start_playback();	// start the audio device playing back
	loop_adjustment = 0;
	if(cursor) cursor->start_playback();   // start the cursor and meters moving
	startup_lock.lock();
	start();
	wait_for_startup();

// Handle frame advance
	if(speed == FRAME_SPEED)
	{
		float frame;
// For frame advance only the frame under the cursor is played back then playback is paused
// before returning to the buttonbar.
		mwindow->gui->unlock_window();
		wait_for_completion();

// Determine next playback range
		frame = (float)(playback_start + 1) / mwindow->sample_rate * mwindow->frame_rate;
		if(reverse)
		{
			playback_start = (long)((frame - 1) / mwindow->frame_rate * mwindow->sample_rate);
			playback_end = (long)(frame / mwindow->frame_rate * mwindow->sample_rate);
			last_position = playback_end;
		}
		else
		{
			playback_start = (long)((frame + 1) / mwindow->frame_rate * mwindow->sample_rate);
			playback_end = (long)((frame + 2) / mwindow->frame_rate * mwindow->sample_rate);
			last_position = playback_start;
		}

// Restart the playback engine.
		arm_playback(follow_loop, 
					use_buttons, 
					infinite, 
					audio);

		mwindow->gui->lock_window();
		if(use_buttons) mwindow->gui->buttonbar->pause_transport();
	}
return 0;
}

int PlaybackEngine::stop_playback(int update_button)
{
	long new_position = 0;
	this->update_button = update_button;

//printf("PlaybackEngine::stop_playback 1\n");
	if(is_playing_back == 2)
	{
// output device is running
		new_position = get_position(0);
//printf("PlaybackEngine::stop_playback 1\n");

// interrupt audio driver forcefully
		if(audio) audio->interrupt_playback();
//printf("PlaybackEngine::stop_playback 1\n");
		if(video) video->interrupt_playback();
// wait for finish
//printf("PlaybackEngine::stop_playback 1\n");
		wait_for_completion();
//printf("PlaybackEngine::stop_playback 2\n");
	}
	else
	if(is_playing_back == 1)
	{
// armed but no thread
		new_position = last_position;
		if(cursor) cursor->stop_playback();
		cleanup();
		reset_buttons();
	}

	is_playing_back = 0;
	if(reverse) playback_end = new_position; else playback_start = new_position;
	last_position = new_position;
return 0;
}

int PlaybackEngine::start_reconfigure()
{
	reconfigure_status = is_playing_back;
	if(is_playing_back) stop_playback(0);
return 0;
}

int PlaybackEngine::stop_reconfigure()
{
	if(reconfigure_status)
	{
		arm_playback(follow_loop,
					use_buttons,
					infinite,
					shared_audio ? audio : 0);

		if(reconfigure_status == 2) start_playback(); // don't start if originally paused
		reconfigure_status = 0;
	}
return 0;
}

int PlaybackEngine::reset_buttons()
{
	if(mwindow->gui && update_button) 
	{
		mwindow->gui->buttonbar->reset_transport();
	}
return 0;
}


long PlaybackEngine::absolute_position(int sync_time)
{
	long result;

	switch(is_playing_back)
	{
		case 2:
// Playing back now.
// try getting from audio device first
			if(audio)
			{
				result = audio->current_position();
			}
			else
// try getting from the video thread only if drawing cursor
			if(video && !sync_time)
			{
				result = (long)((float)render_engine->vrender->absolute_frame / mwindow->frame_rate * mwindow->sample_rate / speed);
			}
			else
// use elapsed time for synchronization
			{
				result = timer.get_scaled_difference(mwindow->sample_rate);
			}
			break;

		case 1:
// Paused
			result = last_position;
			break;
		
		default:
			result = 0;
	}

	return result;
}

long PlaybackEngine::get_position(int sync_time)
{
	long result;

	switch(is_playing_back)
	{
		case 2:
// playing back
			result = absolute_position(sync_time);
// adjust for speed
			result = (long)(result * speed);

// adjust direction and initial position
			if(reverse)
			{
				result = playback_end - result;
				result += loop_adjustment;

// adjust for looping
				while(mwindow->loop_playback && follow_loop && 
					result < mwindow->loop_start)
				{
					result += mwindow->loop_end - mwindow->loop_start;
					loop_adjustment += mwindow->loop_end - mwindow->loop_start;
				}
			}
			else
			{
				result += playback_start;
				result -= loop_adjustment;
				
				while(mwindow->loop_playback && follow_loop && 
					result > mwindow->loop_end)
				{
					result -= mwindow->loop_end - mwindow->loop_start;
					loop_adjustment += mwindow->loop_end - mwindow->loop_start;
				}
			}
			break;

		case 1:
// paused
			result = last_position;
			break;

		default:
// no value
			result = -1;
			break;
	}

	return result;
}


int PlaybackEngine::move_right(long distance) { mwindow->move_right(distance); return 0;
}


int PlaybackEngine::cleanup()
{
	is_playing_back = 0;
	if(!shared_audio && audio)
	{
		delete audio;
		audio = 0;
	}
	
	if(video)
	{
		delete video;
		video = 0;
	}

	if(render_engine) delete render_engine;
	if(cursor) delete cursor;
	render_engine = 0;
	cursor = 0;
return 0;
}

int PlaybackEngine::wait_for_startup()
{
	if(audio) audio->wait_for_startup();
	if(cursor) cursor->wait_for_startup();

	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

int PlaybackEngine::wait_for_completion()
{
// must use mutex not join since playback engine also terminates itself
	complete.lock();
	complete.unlock();
return 0;
}





void PlaybackEngine::run()
{
//printf("PlaybackEngine::run 1\n");fflush(stdout);
	startup_lock.unlock();
//printf("PlaybackEngine::run 1\n");fflush(stdout);
	render_engine->wait_for_completion();
//printf("PlaybackEngine::run 1\n");fflush(stdout);

	reset_buttons();      // change the playbutton to play if there is one
//printf("PlaybackEngine::run 1\n");fflush(stdout);
	if(cursor) cursor->stop_playback();        // kill cursor
//printf("PlaybackEngine::run 1\n");fflush(stdout);

	if(!shared_audio && audio) // delete audio objects if not shared
	{
		audio->close_all();
	}
//printf("PlaybackEngine::run 1\n");fflush(stdout);
	
	if(video) video->close_all();
//printf("PlaybackEngine::run 1\n");fflush(stdout);

// delete more objects
	cleanup();                  
	complete.unlock();             // signal blocking threads to continue
//printf("PlaybackEngine::run 2\n");fflush(stdout);
}
