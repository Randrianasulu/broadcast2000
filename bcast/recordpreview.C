#include <string.h>
#include "previewaudio.h"
#include "previewvideo.h"
#include "record.h"
#include "recordengine.h"
#include "recordpreview.h"

RecordPreview::RecordPreview(Record *record, RecordEngine *record_engine)
{
	this->record = record;
	this->record_engine = record_engine;
}

RecordPreview::~RecordPreview()
{
}

int RecordPreview::initialize()
{
	if(record->do_audio)
		preview_audio = new PreviewAudio(record, record_engine, this);
	if(record->do_video)
		preview_video = new PreviewVideo(record, record_engine, this);
return 0;
}

int RecordPreview::start_preview(long current_position, File *file)
{
	if(record->do_audio)
		preview_audio->start_preview(current_position, file);
	if(record->do_video)
		preview_video->start_preview(current_position, file);

	no_monitor = 0;
	completion_lock.lock();
	Thread::synchronous = 0;
	Thread::start();
	return 0;
return 0;
}

int RecordPreview::stop_preview(int no_monitor)
{
	this->no_monitor = no_monitor;
	if(record->do_audio) preview_audio->stop_preview();
	if(record->do_video) preview_video->stop_preview();
	completion_lock.lock();
	completion_lock.unlock();
	return 0;
return 0;
}

long RecordPreview::absolute_position()
{
	if(record->do_audio) return preview_audio->absolute_position();
	return -1;
}

void RecordPreview::run()
{
	if(record->do_audio) preview_audio->join();
	if(record->do_video) preview_video->join();

	record_engine->is_previewing = 0;
	record_engine->close_output_devices();
	record_engine->resume_monitor();

	completion_lock.unlock();
}
