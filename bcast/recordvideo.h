#ifndef RECORDVIDEO_H
#define RECORDVIDEO_H

#include "file.inc"
#include "mutex.h"
#include "record.inc"
#include "recordgui.inc"
#include "thread.h"
#include "videodevice.inc"

// Default behavior is to read frames and flash to display.
// Optionally writes to disk.

class RecordVideo : public Thread
{
public:
	RecordVideo(Record *record, 
		RecordEngine *record_engine, 
		RecordThread *record_thread,
		VideoDevice *device);
	~RecordVideo();

	void run();
	int start_recording(File *file, int single_frame);
	int stop_recording();
	int pause_recording();
	int resume_recording();
	int wait_for_completion();     // For recording to a file.
	int set_parameters(File *file,
							RecordGUI *gui,
							int buffer_size,    // number of frames to write to disk at a time
							int realtime,
							int frames);
	int unhang_thread();

	Record *record;
	RecordEngine *record_engine;
	RecordThread *record_thread;
	VideoDevice *device;
	RecordGUI *gui;
	File *file;         // 0 if not writing file
	int done;
	int single_frame;
	int buffer_size;    // number of frames to write to disk at a time
	long buffer_position;   // Position in output buffer being captured to
	VFrame *capture_frame;   // Output frame for preview mode
	Timer delayer;
	int result;   // result of a disk write
	int grab_result;  // result of a frame grab
	VFrame ***frame_ptr;
	long current_sample;   // Sample in time of the start of the capture
	long next_sample;      // Sample of next frame
	long total_dropped_frames;  // Total number of frames behind
	long dropped_frames;  // Number of frames currently behind
	long last_dropped_frames;  // Number of dropped frames during the last calculation
	long delay;
	long record_start;     // Frame start of this recording in the file



	int is_recording;
	int is_paused;
	long total_frames;  // Number of frames actually captured
	Mutex unhang_lock;
private:
	int cleanup_recording();
};


#endif
