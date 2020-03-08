#ifndef RENDERENGINE_H
#define RENDERENGINE_H


class RenderEngine;

#include "arender.inc"
#include "audiodevice.inc"
#include "mainwindow.inc"
#include "playbackengine.inc"
#include "videodevice.inc"
#include "vrender.inc"

class RenderEngine
{
public:
// constructing with an audio device forces output buffer allocation
// constructing without an audio device puts in one buffer at a time mode
	RenderEngine(MainWindow *mwindow, 
		AudioDevice *audio = 0, 
		VideoDevice *video = 0,
		PlaybackEngine *playbackengine = 0);
	~RenderEngine();

// buffersize is in samples
	int reset_parameters();

	int arm_playback_common(long start_sample, 
			long end_sample,
			long current_sample,
			int reverse, 
			float speed, 
			int follow_loop,
			int infinite,
			int real_time_playback = 0);

	int arm_playback_audio(long input_length, 
			long amodule_render_fragment, 
			long playback_buffer, 
			long output_length, 
			int output_channels);

	int arm_playback_video(int every_frame, 
			long read_length, 
			long output_length,
			int track_w,
			int track_h,
			int output_w,
			int output_h);

	long absolute_position();    // return absolute position in playback samples
	float get_framestep();
	long get_correction_factor(int reset);     // calling it resets the correction factor

// start video since vrender is the master
	int start_video();
// clean up the audio rendering
	int stop_playback();

	int wait_for_startup();
	int wait_for_completion();

// information for playback
	int reverse;
	float speed;
	int follow_loop;       // loop if mwindow is looped
	int infinite;          // don't stop rendering at the end of the range or loops
	int done;              // flag for threads to stop rendering
	int realtime;          // when an output device exists
	int real_time_playback; // for overriding scheduling priority

	long start_position;      // lowest numbered sample in playback range
	long end_position;        // highest numbered sample in playback range
	long current_sample;
	int output_channels;
	int audio_on;
	int video_on;
	int every_frame;

	MainWindow *mwindow;
	PlaybackEngine *playbackengine;
	AudioDevice *audio;       // devices for output if present
	VideoDevice *video;
	ARender *arender;
	VRender *vrender;
};








#endif
