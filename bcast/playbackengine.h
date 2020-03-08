#ifndef PLAYBACKENGINE_H
#define PLAYBACKENGINE_H

#include "audiodevice.inc"
#include "defaults.inc"
#include "mainwindow.inc"
#include "maxchannels.h"
#include "modules.inc"
#include "mutex.h"
#include "playbackcursor.inc"
#include "renderengine.inc"
#include "thread.h"
#include "timer.h"
#include "videodevice.inc"

class PlaybackEngine : public Thread
{
public:
	PlaybackEngine(MainWindow *mwindow);
	~PlaybackEngine();

	int wait_for_startup();
	int wait_for_completion();

	int reset_parameters();

// arm the first buffers
	int set_range(long start,
				long end,
				long current,
				int reverse,
				float speed);

	int arm_playback(int follow_loop, 
				int use_buttons, 
				int infinite = 0,
				AudioDevice *audio = 0);

// start the thread
	int start_playback();
// interrupt playback
	int stop_playback(int update_button = 0);

// ================= position information ======================
// get exact position in samples corrected for speed and direction
	long get_position(int sync_time = 1);
// get total samples rendered since last start with no speed correction
// Sync_time uses the video thread to get the current position.
// Otherwise a timer or audio device is used.
	long absolute_position(int sync_time = 1);

// stop and start for user changes
	int start_reconfigure();
	int stop_reconfigure();

// generic render routine
	int render_audio();
	int reset_buttons();
	int cleanup();
	void run();
	Mutex complete, startup_lock;

// ============================ cursor
	int lock_playback_cursor();
	int unlock_playback_cursor();
	int lock_playback_movement();
	int unlock_playback_movement();
	int move_right(long distance);

	PlaybackCursor *cursor;


// ============================== playback config
	int infinite;        // for infinite playback	
	int follow_loop;     // 1 if mwindow's looping setting is followed
	int is_playing_back;    // 0 - no playback  1 - armed but stopped  2 - playing back
	int reconfigure_status;    // 0 - no playback  1 - armed but stopped  2 - playing back
							      // otherwise use DSP chip if audio tracks are being played
	int update_button;            // flag for thread on exit
	int use_buttons;              // flag to update buttons when done or not
	long last_position;         // starting position for restarting playback
	long playback_start, playback_end;  // range for current playback
	int reverse;                // direction of current playback
	float speed;                // speed of current playback.  A speed of FRAME_SPEED causes frame advance
	long playback_buffer;
	long audio_module_fragment;

// ================================== audio config
	long input_length;        // number of samples to read from disk at a time
						      // multiple of playback_buffer greater than read_buffer
	long output_length;       // # of samples to write to device adjusted for speed
	int shared_audio;    // for duplex audio recording

// ================================== video config

	long correction_factor;   // number of frames to skip to get synchronized

	RenderEngine *render_engine;
	AudioDevice *audio;
	VideoDevice *video;
	MainWindow *mwindow;
	Modules *modules;

private:
	int init_parameters();
	int init_cursor();
	int init_audio_device();
	int init_video_device();
	Timer timer;    // timer for position information
	long loop_adjustment;       // for factoring loops into current position
};





#endif
