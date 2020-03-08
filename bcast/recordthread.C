#include <string.h>
#include "record.h"
#include "recordaudio.h"
#include "recordengine.h"
#include "recordthread.h"
#include "recordvideo.h"


RecordThread::RecordThread(Record *record, RecordEngine *record_engine)
 : Thread()
{
	this->record = record;
	this->record_engine = record_engine;
	quit_when_completed = 0;
}

RecordThread::~RecordThread()
{
}

int RecordThread::initialize()
{
	if(record->do_audio) 
		record_audio = new RecordAudio(record, 
			record_engine, 
			this, 
			record_engine->adevice);
	if(record->do_video) 
		record_video = new RecordVideo(record, 
			record_engine, 
			this, 
			record_engine->vdevice);
	done = 0;
	return 0;
return 0;
}

int RecordThread::start_recording(File *file, int single_frame)
{
	this->file = file;
	this->single_frame = single_frame;
	done = 1;
	no_monitor = 0;

	loop_lock.lock();
	startup_lock.lock();
	completion_lock.lock();
	Thread::synchronous = 0;
	Thread::start();
	startup_lock.lock();
	startup_lock.unlock();
	return 0;
return 0;
}

int RecordThread::stop_recording(int no_monitor)
{
	this->no_monitor = no_monitor;
	done = 1;
	if(record->do_audio && !single_frame)
		record_audio->stop_recording();
	if(record->do_video)
		record_video->stop_recording();
	
	completion_lock.lock();
	completion_lock.unlock();
	return 0;
return 0;
}

int RecordThread::pause_recording()
{
	done = 0;
// Stop the thread
	pause_lock.lock();

// Stop the recordings
	if(record->do_audio)
		record_audio->stop_recording();
	if(record->do_video)
		record_video->stop_recording();

// Wait for thread to stop
	loop_lock.lock();
	loop_lock.unlock();
	record_engine->close_input_devices();
	return 0;
return 0;
}

int RecordThread::resume_recording()
{
	loop_lock.lock();
	pause_lock.unlock();
	return 0;
return 0;
}

long  RecordThread::absolute_position()
{
	if(record->do_audio) return record_audio->absolute_position();
	return -1;
}


void RecordThread::run()
{
	do
	{
		if(record->do_audio && !single_frame)
			record_audio->start_recording(file);

		if(record->do_video)
			record_video->start_recording(file, single_frame);

		startup_lock.unlock();

		if(record->do_audio && !single_frame)
			record_audio->join();
		if(record->do_video)
			record_video->join();

		loop_lock.unlock();
		pause_lock.lock();
		pause_lock.unlock();
	}while(!done && !no_monitor);

// Resume monitoring only if not a monitor ourselves
	if(file)
	{
		record_engine->stop_duplex();
		record_engine->close_input_devices();
		record_engine->is_saving = 0;
		if(!no_monitor) record_engine->resume_monitor();
	}
	else
	{
		record_engine->close_input_devices();
		record_engine->is_monitoring = 0;
	}
	completion_lock.unlock();
}
