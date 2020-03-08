#include <string.h>
#include "assets.h"
#include "file.h"
#include "pluginbuffer.h"
#include "previewvideo.h"
#include "record.h"
#include "recordgui.h"
#include "recordengine.h"
#include "recvideowindow.h"
#include "timer.h"
#include "vframe.h"
#include "vrender.inc"   // want the VRENDER_THRESHOLD

PreviewVideo::PreviewVideo(Record *record, 
	RecordEngine *record_engine, 
	RecordPreview *preview_thread)
 : Thread()
{
	this->record = record;
	this->record_engine = record_engine;
	this->preview_thread = preview_thread;
}

PreviewVideo::~PreviewVideo()
{
}

int PreviewVideo::start_preview(long position, File *file)
{
	this->file = file;
	this->gui = record_engine->gui;
	this->current_position = (long)toframes(position, record->get_samplerate(), record->get_framerate());
	this->preview_start = current_position;
	done = 0;

	preview_end = file->get_video_length(record->get_framerate());

	Thread::synchronous = 1;
	Thread::start();
	return 0;
return 0;
}

int PreviewVideo::stop_preview()
{
	done = 1;
	return 0;
return 0;
}

void PreviewVideo::run()
{
	long skip_countdown = VRENDER_THRESHOLD;
	long delay_countdown = VRENDER_THRESHOLD;
	long current_sample, start_sample, end_sample;
	long delay, skip;
	PluginBuffer output_frame_buffer(record->frame_w * record->frame_h, 3);
	VFrame output_frame((unsigned char*)output_frame_buffer.get_data(), 
				record->frame_w, 
				record->frame_h, 
				VFRAME_RGB888);

	Timer timer;
	long last_frame = -1;

// preview engine
	while(!done &&
		current_position < preview_end && 
		current_position > last_frame)
	{
// Perform the most time consuming part of frame decompression now.
		file->lock_read();
		file->set_video_position(current_position, record->get_framerate());
		file->read_raw_frame(&output_frame, &output_frame_buffer, 0);
		file->unlock_read();
		last_frame = current_position;
		if(!record->do_audio) 
		{
			record_engine->update_position(tosamples(current_position, record->get_samplerate(), record->get_framerate()));
		}

// Determine the delay until the frame needs to be shown.
		current_sample = (long)(record_engine->absolute_preview_position());
// latest sample by which the frame needs to be shown.
		end_sample = tosamples(current_position - preview_start, record->get_samplerate(), record->get_framerate());
// earliest sample at which the frame can be shown.
		start_sample = tosamples(current_position - preview_start - 1, record->get_samplerate(), record->get_framerate());

// Current sample is -1 is the timer isn't running.
		if(end_sample < current_sample || current_sample < 0)
		{
// Frame rendered late.  Flash it now.
			gui->monitor_video_window->update(&output_frame, PLAYBACK_X11);

//printf("rendered late ");
			if(record->get_everyframe())
			{
// User wants every frame.
				current_position++;
			}
			else
			if(skip_countdown > 0)
			{
// Maybe just a freak.
//printf("skip_countdown %d\n", skip_countdown);
				current_position++;
				skip_countdown--;
			}
			else
			{
// Get the frames to skip.
				delay_countdown = VRENDER_THRESHOLD;
				skip = 1 + (long)toframes(current_sample, record->get_samplerate(), record->get_framerate())
					 - (long)toframes(end_sample, record->get_samplerate(), record->get_framerate());
//printf("skip %d ***\n", skip);
				current_position += skip;
			}
		}
		else
		{
// Frame rendered early or just in time.
			current_position++;

//printf("rendered early ");
			if(delay_countdown > 0)
			{
// Maybe just a freak
//printf("delay_countdown %d\n", delay_countdown);
				delay_countdown--;
			}
			else
			{
				skip_countdown = VRENDER_THRESHOLD;
				if(start_sample > current_sample)
				{
// Came before the earliest sample so delay
					delay = (long)((float)(start_sample - current_sample) * 1000 / record->get_samplerate());
//printf("delay %d\n", delay);
					timer.delay(delay);
				}
				else
				{
// Came after the earliest sample so keep going
//printf("before earliest\n", delay);
				}
			}

// Flash frame now.
			gui->monitor_video_window->update(&output_frame, PLAYBACK_X11);
		}
	}

	done = 1;

	if(!record->do_audio) file->set_video_position(current_position, record->get_framerate());
}
