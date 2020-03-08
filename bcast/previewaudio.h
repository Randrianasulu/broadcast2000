#ifndef PREVIEWAUDIO_H
#define PREVIEWAUDIO_H

#include "audiodevice.inc"
#include "bcbase.h"
#include "file.inc"
#include "maxchannels.h"
#include "mutex.h"
#include "pluginbuffer.inc"
#include "record.inc"
#include "recordengine.inc"
#include "recordpreview.inc"
#include "thread.h"




class PreviewAudio : public Thread
{
public:
	PreviewAudio(Record *record, 
		RecordEngine *record_engine, 
		RecordPreview *preview_thread);
	~PreviewAudio();

	int start_preview(long position, File *file);
	int stop_preview();



	int arm_buffer();
	int update_current_position();
	long get_current_position();
	long absolute_position();

	void run();

// meter information
	File *file;
	Record *record;
	RecordEngine *record_engine;
	RecordPreview *preview_thread;
	int fragments;
	int total_peaks;
	float **peak_history;
	long *peak_sample;
	int current_peak, output_peak, last_peak;
	BC_Meter **meter;
	AudioDevice *device;
	long buffer_size, fragment_size;
	long preview_start, current_position, preview_end;
	long last_position;
	int done;

	PluginBuffer *shared_output[MAXCHANNELS];
	float *output[MAXCHANNELS];
	int playback_channels;
};


class PreviewPosition : public Thread
{
public:
	PreviewPosition(PreviewAudio *engine);
	~PreviewPosition();

	void run();
	PreviewAudio *thread;
};

#endif
