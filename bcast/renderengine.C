#include <string.h>
#include "arender.h"
#include "audiodevice.h"
#include "mainwindow.h"
#include "playbackengine.h"
#include "preferences.h"
#include "renderengine.h"
#include "videodevice.h"
#include "vrender.h"

RenderEngine::RenderEngine(MainWindow *mwindow, 
		AudioDevice *audio, 
		VideoDevice *video,
		PlaybackEngine *playbackengine)
{
	reset_parameters();
	this->mwindow = mwindow;
	this->playbackengine = playbackengine;
	this->audio = audio;
	this->video = video;
	if(audio || video) realtime = 1;
}

RenderEngine::~RenderEngine()
{
}

int RenderEngine::reset_parameters()
{
	realtime = 0;
	start_position = 0;
	follow_loop = 0;
	end_position = 0;
	infinite = 0;
	speed = 0;
	reverse = 0;
	start_position = 0;
	output_channels = 0;
	audio_on = 0;
	video_on = 0;
	done = 0;
return 0;
}

int RenderEngine::arm_playback_common(long start_sample, 
			long end_sample,
			long current_sample,
			int reverse, 
			float speed, 
			int follow_loop,
			int infinite,
			int real_time_playback)
{
	this->follow_loop = follow_loop;
	this->start_position = start_sample;
	this->end_position = end_sample;
	this->speed = speed;
	this->reverse = reverse;
	this->infinite = infinite;
	this->current_sample = current_sample;
	this->real_time_playback = real_time_playback;
	if(infinite) this->follow_loop = 0;
return 0;
}

int RenderEngine::arm_playback_audio(long input_length, 
			long amodule_render_fragment, 
			long playback_buffer, 
			long output_length, 
			int output_channels)
{
	this->output_channels = output_channels;

	audio_on = 1;

	arender = new ARender(mwindow, this);
	arender->arm_playback(current_sample, 
							input_length, 
							amodule_render_fragment, 
							playback_buffer, 
							output_length);
return 0;
}

int RenderEngine::arm_playback_video(int every_frame, 
			long read_length, 
			long output_length,
			int track_w,
			int track_h,
			int output_w,
			int output_h)
{
	video_on = 1;
	this->every_frame = every_frame;
	
	vrender = new VRender(mwindow, this);
	vrender->arm_playback(current_sample, 
							read_length, 
							output_length, 
							output_length, 
							track_w,
							track_h,
							output_w,
							output_h);
return 0;
}

int RenderEngine::start_video()
{
// start video for realtime
	if(video) video->start_playback();
	vrender->start_playback();
return 0;
}

int RenderEngine::stop_playback()
{
	if(audio_on || video_on)
	{
		follow_loop = 0;
		reverse = 0;
		speed = 0;
	}

	if(audio_on)
	{
		arender->stop_rendering();
		delete arender;
	}

	if(video_on)
	{
		vrender->stop_rendering();
		delete vrender;
	}

	audio_on = 0;
	video_on = 0;
return 0;
}

float RenderEngine::get_framestep()
{
	if(every_frame)
		return 1;
	else
		return mwindow->frame_rate / mwindow->preferences->actual_frame_rate;
}

long RenderEngine::absolute_position()
{
	return playbackengine->absolute_position(1);
}

long RenderEngine::get_correction_factor(int reset)
{
	if(!every_frame)
	{
		long x;
		x = playbackengine->correction_factor;
		if(reset) playbackengine->correction_factor = 0;
		return x;
	}
	else
		return 0;
}

int RenderEngine::wait_for_startup()
{
	if(audio_on) arender->wait_for_startup();
	if(video_on) vrender->wait_for_startup();
return 0;
}

int RenderEngine::wait_for_completion()
{
//printf("RenderEngine::wait_for_completion 1\n");
	if(audio_on)
	{
		arender->wait_for_completion();
		delete arender;
		audio_on = 0;
	}
//printf("RenderEngine::wait_for_completion 1\n");

	if(video_on)
	{
		vrender->wait_for_completion();
		delete vrender;
		video_on = 0;
	}
//printf("RenderEngine::wait_for_completion 2\n");
return 0;
}
