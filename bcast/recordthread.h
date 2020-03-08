#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#include "file.inc"
#include "record.inc"
#include "recordaudio.inc"
#include "recordengine.inc"
#include "recordvideo.inc"
#include "thread.h"

// Synchronously handle recording and monitoring

class RecordThread : public Thread
{
public:
	RecordThread(Record *record, RecordEngine *record_engine);
	~RecordThread();

	int initialize();
	int start_recording(File *file, int single_frame = 0);
	int stop_recording(int no_monitor = 0);
	int pause_recording();
	int resume_recording();
	long absolute_position();
	
	void run();

	int quit_when_completed;
	RecordAudio *record_audio;
	RecordVideo *record_video;

private:
	Record *record;
	RecordEngine *record_engine;
	File *file;
	Mutex pause_lock, startup_lock, completion_lock, loop_lock;
	int done;
	int single_frame;
	int no_monitor;
};


#endif
