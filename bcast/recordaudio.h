#ifndef RECORDAUDIO_H
#define RECORDAUDIO_H

#include "audiodevice.inc"
#include "bcbase.h"
#include "file.inc"
#include "mutex.h"
#include "record.inc"
#include "recordgui.inc"
#include "recordengine.inc"
#include "recordthread.inc"
#include "thread.h"


class RecordAudio : public Thread
{
public:
	RecordAudio(Record *record, 
		RecordEngine *record_engine, 
		RecordThread *record_thread, 
		AudioDevice *device);
	~RecordAudio();
	
	void run();
	int start_recording(File *file);            // start saving audio data to file
	int stop_recording();

	long get_position();
	int set_position(long position);     // seek to a new location in the file 
	long absolute_position();

	int write_buffer();           // write the buffer
	int done;      // Allow video recorder to stop the audio recorder

private:
	Record *record;
	RecordEngine *record_engine;
	RecordThread *record_thread;
	AudioDevice *device;
	File *file;         // 0 if not writing file
	long total_samples;
	float *max;
	int *over;
	float **input;
	RecordGUI *gui;
	BC_Meter **meter;
	long buffer_size, fragment_size, fragment_position;
	int record_channels;
	Mutex timer_lock;
	Timer timer;
};



#endif
