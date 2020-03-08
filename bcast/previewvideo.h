#ifndef PREVIEWTHREAD_H
#define PREVIEWTHREAD_H

#include "bcbase.h"
#include "file.inc"
#include "mutex.h"
#include "record.inc"
#include "recordgui.inc"
#include "recordengine.inc"
#include "recordpreview.inc"
#include "thread.h"




class PreviewVideo : public Thread
{
public:
	PreviewVideo(Record *record, 
		RecordEngine *record_engine, 
		RecordPreview *preview_thread);
	~PreviewVideo();

	int start_preview(long position, File *file);
	int stop_preview();

	int arm_buffer();

	void run();

	File *file;
	Record *record;
	RecordEngine *record_engine;
	RecordPreview *preview_thread;
	RecordGUI *gui;
	long preview_start, current_position, preview_end;
	int done;
};

#endif
