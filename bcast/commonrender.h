#ifndef COMMONRENDER_H
#define COMMONRENDER_H

#include "commonrenderthread.inc"
#include "mainwindow.inc"
#include "renderengine.inc"
#include "thread.h"

class CommonRender : public Thread
{
public:
	CommonRender(MainWindow *mwindow, RenderEngine *renderengine);
	~CommonRender();

// clean up rendering
	int virtual stop_rendering() { return 0; };
	int wait_for_completion();
	virtual int wait_device_completion() { return 0; };
// renders to a device when there's a device
	virtual int process_buffer(long input_len, long input_position) { return 0; };
	virtual int restart_playback() { return 0; };          // restart for playback automation changes

	virtual int get_datatype() { return 0; };
// test region against loop boundaries
	int get_boundaries(long &current_render_length);
// test region for playback automation changes
	int get_automation(long &current_render_length, int data_type);
// advance the buffer position depending on the loop status
	int advance_position(long current_render_length);

// convert to and from the native units of the render engine
	virtual long tounits(long position);
	virtual long fromunits(long position);
	virtual long get_render_length(long current_render_length) { return 0; };

	MainWindow *mwindow;
	RenderEngine *renderengine;
	CommonRenderThread *thread;
	long current_position;       // position in project used for all functions

	long input_length;           // frames/samples to read from disk at a time
	int interrupt;  // flag for interrupted playback
	int done;       // flag for normally completed playback
	int last_playback;  // flag for last buffer to be played back
	int asynchronous;     // if this media type is being rendered asynchronously by threads

protected:
// make sure automation agrees with playable tracks
// automatically tests direction of playback
// return 1 if it doesn't
	int test_automation_before(long &current_render_length, int data_type);
	int test_automation_after(long &current_render_length, int data_type);
	int test_virtualnodes(long current_position, long &current_input_length, int data_type, int reverse);
};


#endif
