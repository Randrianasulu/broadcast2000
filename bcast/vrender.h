#ifndef VRENDER_H
#define VRENDER_H

#include "bcbase.h"
#include "commonrender.h"
#include "edit.inc"
#include "mainwindow.inc"
#include "pluginbuffer.inc"
#include "renderengine.inc"
#include "vframe.inc"


class VRender : public CommonRender
{
public:
	VRender(MainWindow *mwindow, RenderEngine *renderengine);
	~VRender();

// set up and start thread
	int arm_playback(long current_position,
				long input_length, 
				long module_render_fragment, 
				long playback_buffer, 
				int track_w,
				int track_h,
				int output_w,
				int output_h);

	void run();
	int wait_for_startup();

	int start_playback();     // start the thread
	int restart_playback();          // restart for playback automation changes

// get data type for certain commonrender routines
	int get_datatype();

// process frames to put in buffer_out
	int process_buffer(VFrame **buffer_out, long input_len, long input_position, int last_buffer);
// load an array of buffers for each track to send to the thread
	int process_buffer(long input_len, long input_position);
// move the pointers to the frames to reverse order
	int reverse_buffer(VFrame **buffer, long len, long step);
// Flash the output on the display
	int flash_output();
// Determine if data can be copied directly from the file to the output device.
	int get_render_strategy(Edit* &playable_edit, long input_position);

// clean up rendering
	int stop_rendering();

	long tounits(long position);
	long fromunits(long position);
	long get_render_length(long current_render_length);

	long absolute_frame;           // absolute frame being rendered

// console dimensions
	int track_w, track_h;    
// video device dimensions
	int output_w, output_h;    
// frames to send to console fragment
	long vmodule_render_fragment;    
// frames to send to video device (1)
	long playback_buffer;            
// Output frame use by video device
	VFrame **video_out;
// Shared memory for output frame
	PluginBuffer *video_out_buffer;    
// Byte offset of video_out in video_out_buffer
	long video_out_byte;    
// current frame in output buffer being written (0)
	long video_out_position;       
	long source_length;  // Total number of frames to render for transitions

private:
	int init_device_buffers();
	Timer timer;

// for getting actual framerate
	long framerate_counter;
	Timer framerate_timer;
	int render_strategy;
};




#endif
