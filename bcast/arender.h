#ifndef ARENDER_H
#define ARENDER_H

#include "commonrender.h"
#include "mainwindow.inc"
#include "mutex.h"

class ARender : public CommonRender
{
public:
	ARender(MainWindow *mwindow, RenderEngine *renderengine);
	~ARender();

// set up and start thread
	int arm_playback(long current_position,
				long input_length, 
				long module_render_fragment, 
				long playback_buffer, 
				long output_length);
	int wait_for_startup();

	void run();

// get the data type for certain commonrender routines
	int get_datatype();

// handle playback autos and transitions
	int restart_playback();
	int send_reconfigure_buffer();

// process a buffer
// renders into buffer_out argument when no audio device
// handles playback autos
	int process_buffer(float **buffer_out, long input_len, long input_position, int last_buffer);
// renders to a device when there's a device
	int process_buffer(long input_len, long input_position);

// clean up rendering
	int stop_rendering();
	int wait_device_completion();

// reverse the data in a buffer	
	int reverse_buffer(float *buffer, long len);
// advance the buffer count
	int swap_current_buffer();
	long get_render_length(long current_render_length);

	int total_out_buffers;   // output channels

// exit conditions

	long amodule_render_fragment;    // maximum length to send to console fragment
	long playback_buffer;            // maximum length to send to audio device
	long output_length;              // maximum length to send to audio device after speed

// information for meters
	int total_peaks;         // peaks for module fragments
	int current_peak;        // number of current peak
	long *peak_samples;      // samples of peaks
	int get_next_peak(int current_peak);
	long meter_render_fragment;    // samples to use for one meter update

	long source_length;  // Total number of frames to render for transitions

// output buffer for audio device
	float **audio_out;
// current position in output buffer before speed adjustment
	long audio_out_position;

private:
// initialize buffer_out
	int init_device_buffers();
	int init_meters();
	long absolute_position;  // Position for meters
	Mutex startup_lock;
};



#endif
