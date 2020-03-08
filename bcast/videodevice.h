#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include "assets.inc"
#include "audio1394.inc"
#include "audiodevice.inc"
#include "bcbase.h"
#include "bccapture.inc"
#include "channel.inc"
#include "mainwindow.inc"
#include "mutex.h"
#include "recvideowindow.inc"
#include "thread.h"
#include "timer.h"
#include "vdevicebase.inc"
#include "vdevice1394.inc"
#include "vdevicelml.inc"
#include "vdevicex11.inc"
#include "videoconfig.inc"
#include "videowindow.inc"

#include "videodev2.h"

// The keepalive thread runs continuously during recording.
// If the recording thread doesn't reset still_alive, failed is incremented.
// Failed is set to 0 if the recording thread resets still_alive.

// Number of seconds for keepalive to freak out
#define KEEPALIVE_DELAY 0.5

// Structure for video4linux2
struct tag_vimage
{
	struct v4l2_buffer vidbuf;
	char *data;
};

class VideoDevice;

class KeepaliveThread : public Thread
{
public:
	KeepaliveThread(VideoDevice *device);
	~KeepaliveThread();

	void run();
	int reset_keepalive();   // Call after frame capture to reset counter
	int get_failed();
	int start_keepalive();
	int stop();

	Timer timer;
	int still_alive;
	int failed;
	int interrupted;
	VideoDevice *device;
	Mutex startup_lock;
	int capturing;
};

class VideoDevice
{
public:
// Recording constructor
	VideoDevice();
// Playback constructor
	VideoDevice(MainWindow *mwindow);
	~VideoDevice();

	friend VDeviceLML;
	friend VDeviceX11;
	friend VDevice1394;
	friend Audio1394;

	int close_all();

// ===================================== Recording
	int open_input(VideoConfig *config, 
		int frame_w, 
		int frame_h, 
		int input_x, 
		int input_y, 
		float input_z,
		float frame_rate);
// Unlocks the device if being shared with audio
	int stop_sharing();
// Specify the audio device opened concurrently with this video device
	int set_adevice(AudioDevice *adevice);
// Called by the audio device to share a buffer
	int get_shared_data(unsigned char *data, long size);
// Return 1 if capturing locked up
	int get_failed();  
// Interrupt a crashed DV device
	int interrupt_crash();
// Schedule capture size to be changed.
	int set_translation(int input_x, int input_y, float input_z);
// Change the channel
	int set_channel(Channel *channel);
// Change field order
	int set_field_order(int odd_field_first);
// Set frames to clear after translation change.
	int set_latency_counter(int value);
// Values from -100 to 100
	int set_picture(int brightness, 
		int hue, 
		int color, 
		int contrast, 
		int whiteness);
	int capture_frame(int frame_number);  // Start the frame_number capturing
	int read_buffer(VFrame *frame);  // Read the next frame off the device
	int frame_to_vframe(VFrame *frame, unsigned char *input); // Translate the captured frame to a VFrame
	int initialize();
	ArrayList<char *>* get_inputs();
	BC_Bitmap* get_bitmap();

	float orate, irate;               // frame rates
	Timer buffer_timer;               // timer for displaying frames in the current buffer
	Timer rate_timer;                 // timer for getting frame rate
	int out_w, out_h;                 // size of output frame being fed to device during playback
	int in_w, in_h;                   // size of frame being read from device during recording
	int r, w;                         // modes
	long frame_delay;                 // time from start of previous frame to start of next frame in ms
	MainWindow *mwindow;              //
	int is_recording; // status of thread
	float frame_rate; // Frame rate to set in device
// Location of input frame in captured frame
	int frame_in_capture_x1, frame_in_capture_x2, frame_in_capture_y1, frame_in_capture_y2;
	int capture_in_frame_x1, capture_in_frame_x2, capture_in_frame_y1, capture_in_frame_y2;
// Size of raw captured frame
	int capture_w, capture_h;
	int input_x, input_y;
	float input_z;
// Captured frame size can only be changed when ready
	int new_input_x, new_input_y;
	float new_input_z;
	int frame_resized;
// When the frame is resized, need to clear all the buffer frames.
	int latency_counter;
	int capturing;
	int swap_bytes;
// All the input sources on the device
	ArrayList<char *> input_sources;
	int odd_field_first;


// ================================== Playback
	int open_output(VideoConfig *config, 
					float rate, 
					int out_w, 
					int out_h, 
					int output_format,  // Color model
					int slippery);  // If slippery is 1 the output is copied to a temp and rendered in the background
// Slippery is only used for hardware compression drivers
	int start_playback();
	int interrupt_playback();
	int wait_for_startup();
	int wait_for_completion();
	int output_visible();     // Whether the output is visible or not.
	int stop_playback();
	long current_position();     // last frame rendered
	int write_buffer(VFrame *output);   // absolute frame of last frame in buffer
// Get render strategies from the device
	ArrayList<int>* get_render_strategies(); 

	int interrupt;
// Compression format in use by the output device
	int output_format;   
	int slippery;
	int is_playing_back;
// Audio device to share data with
	AudioDevice *adevice;
// Reading data from the audio device
	int sharing;
// Synchronize the close devices
	int done_sharing;
	Mutex sharing_lock;

private:
	int set_cloexec_flag(int desc, int value);
	int set_mute(int muted);
// Next frame to capture
	int next_frame(int previous_frame);   
// Change the capture size when ready
	int update_translation();  

	int init_video4linux();  // Set the frame size and parameters for video4linux capturing
	int init_video4linux2();  // Set the frame size and parameters for video4linux capturing
	int unmap_v4l_shmem();
	int unmap_v4l2_shmem();
	int close_v4l();
	int close_v4l2();
	int wait_v4l_frame();
	int wait_v4l2_frame();
	int read_v4l_frame(VFrame *frame);
	int read_v4l2_frame(VFrame *frame);
	int v4l1_get_inputs();       // Determine what inputs are available on the device
	int v4l1_set_channel(Channel *channel);
	int v4l1_get_norm(int norm);  // Get the #define from the norm
	int v4l1_set_mute(int muted);
	int v4l1_set_picture(int brightness, 
		int hue, 
		int color, 
		int contrast, 
		int whiteness);


	VDeviceBase *input_base;
	VDeviceBase *output_base;
	VideoConfig *in_config, *out_config;

// Video4Linux
	struct video_window window_params;
	struct video_picture picture_params;
	struct video_mbuf capture_params;  // Capture for Video4Linux

// Video4Linux 2
	struct v4l2_requestbuffers v4l2_buffers;
	struct v4l2_format v4l2_params;
	struct tag_vimage *v4l2_buffer_list;
	struct v4l2_streamparm v4l2_parm;

// Common
	int input_fd, output_fd;
	char *capture_buffer;      // sequentual capture buffers for v4l1 or read buffer for v4l2
	int capture_frame_number;    // number of frame to capture into
	int read_frame_number;       // number of the captured frame to read
	int shared_memory;   // Capturing directly to memory
	KeepaliveThread *keepalive;

// Screen capture
	BC_Capture *capture_bitmap;
};



#endif
