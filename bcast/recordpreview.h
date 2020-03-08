#ifndef RECORDPREVIEW_H
#define RECORDPREVIEW_H


#include "mutex.h"
#include "previewaudio.inc"
#include "previewvideo.inc"
#include "record.inc"
#include "recordengine.inc"
#include "mutex.h"
#include "thread.h"

// Synchronously handle previewing recordings

class RecordPreview : public Thread
{
public:
	RecordPreview(Record *record, RecordEngine *record_engine);
	~RecordPreview();
	
	int initialize();
	int start_preview(long current_position, File *file);
	int stop_preview(int no_monitor);
	long absolute_position();
	void run();

private:
	PreviewAudio *preview_audio;
	PreviewVideo *preview_video;
	Record *record;
	RecordEngine *record_engine;
	Mutex completion_lock;
	int no_monitor;
};



#endif
