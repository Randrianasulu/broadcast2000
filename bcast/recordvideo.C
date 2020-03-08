#include <string.h>
#include "errorbox.h"
#include "file.h"
#include "record.h"
#include "recordaudio.h"
#include "recordengine.h"
#include "recordgui.h"
#include "recordthread.h"
#include "recordvideo.h"
#include "recvideowindow.h"
#include "vframe.h"
#include "videodevice.h"

#include <unistd.h>

RecordVideo::RecordVideo(Record *record, 
	RecordEngine *record_engine, 
	RecordThread *record_thread,
	VideoDevice *device)
 : Thread()
{
	this->record = record;
	this->record_engine = record_engine;
	this->record_thread = record_thread; 
	this->device = device;
}

RecordVideo::~RecordVideo()
{
}

int RecordVideo::start_recording(File *file, int single_frame)
{
	this->single_frame = single_frame;
	this->file = file;
	gui = record_engine->gui;
	buffer_size = record->get_video_buffersize();

	total_frames = 0;
	done = 0;

	result = 0;
	total_dropped_frames = 0;
	dropped_frames = 0;
	last_dropped_frames = 0;
	record_start = 0;
	buffer_position = 0;

	Thread::synchronous = 1;
	Thread::start();

	return 0;
return 0;
}

int RecordVideo::stop_recording()
{
	done = 1;

// Interrupt DV crashes
	device->interrupt_crash();

// Interrupt video4linux crashes
	if(device->get_failed())
	{
		Thread::end();
		Thread::join();

		cleanup_recording();
	}
	else
	{
		//Thread::join();
	}
	return 0;
return 0;
}


int RecordVideo::cleanup_recording()
{
	if(file)
	{
// write last buffer
		result = file->write_video_buffer(buffer_position, 
					0, 
					record->use_floatingpoint(),
					(record->get_video_driver() == CAPTURE_LML ||
						record->get_video_driver() == CAPTURE_FIREWIRE));
// stop file I/O
		file->stop_video_thread();
	}
	else
	{
		delete frame_ptr[0];
		delete frame_ptr;
		delete capture_frame;
	}
	return 0;
return 0;
}


void RecordVideo::run()
{
	result = 0;
	grab_result = 0;

// Thread out the I/O
	if(file)
	{
		record_start = file->get_video_position(record->get_frame_rate());
		file->start_video_thread(buffer_size, 
							0, 
							2, 
							(record->get_video_driver() == CAPTURE_LML ||
							record->get_video_driver() == CAPTURE_FIREWIRE));
		frame_ptr = file->get_video_buffer();
	}
	else
	{
		capture_frame = new VFrame(0, record->frame_w, record->frame_h, VFRAME_RGB888);
		frame_ptr = new VFrame**[1];
		frame_ptr[0] = new VFrame*[1];
		frame_ptr[0][0] = capture_frame;
	}

// Number of frames for user to know about.
	gui->total_dropped_frames = 0;
	gui->update_dropped_frames(0);

	while(!done && !result)
	{
		if(buffer_position >= buffer_size)
		{
			frame_ptr = file->get_video_buffer();
			buffer_position = 0;
		}

// ============================= Synchronize with audio or timer
		if(record->monitor_video || file)
		{
			next_sample = (long)((float)total_frames / 
				record->get_framerate() * 
				record->get_samplerate());

 			if(file) 
				current_sample = record_engine->absolute_record_position();
 			else
 				current_sample = record_engine->absolute_monitor_position();

			if(current_sample < next_sample && current_sample > 0)
			{
// Too early.
				delay = (long)((float)(next_sample - current_sample) / 
					record->get_samplerate() * 
					1000);
// Sanity check and delay.
				if(delay < 10000 && delay > 0) delayer.delay(delay);
				gui->update_dropped_frames(0);
				last_dropped_frames = 0;
			}
			else
			if(current_sample > 0 && file)
			{
// Captured too slow.
				dropped_frames = (long)((float)(current_sample - next_sample) / 
					record->get_samplerate() * 
					record->get_framerate());
				if(dropped_frames != last_dropped_frames)
				{
					gui->update_dropped_frames(dropped_frames);
					last_dropped_frames = dropped_frames;
				}
				last_dropped_frames = dropped_frames;
			}
		}

// ================================ Capture a frame
		if(!done && (record->monitor_video || file))
		{
			if(file)
			{
				capture_frame = frame_ptr[0][buffer_position];
				record->video_lock.lock();
				device->set_field_order(record->reverse_interlace);
				grab_result = device->read_buffer(capture_frame);
				record->video_lock.unlock();
				if(!grab_result) buffer_position++;
			}
			else
			{
				device->set_field_order(record->reverse_interlace);
				grab_result = device->read_buffer(capture_frame);
			}

			if(file && !record->do_audio) 
				record_engine->update_position(tosamples(record_start + total_frames, 
					record->get_samplerate(), 
					record->get_framerate()));
		}

// Flash the frame
		if(!grab_result && record->monitor_video && !done)
			gui->monitor_video_window->update(capture_frame, record->get_video_driver());

// Compress a batch of frames
		if(file && buffer_position >= buffer_size)
		{
			result = file->write_video_buffer(buffer_position, 
						0, 
						record->use_floatingpoint(),
						(record->get_video_driver() == CAPTURE_LML ||
							record->get_video_driver() == CAPTURE_FIREWIRE));
		}

// Delay if no monitoring and no compressing
		if(!record->monitor_video && !file)
			delayer.delay(250);

		if(file)
		{
// handle different recording modes
			if(!done &&
				!result &&
				record_engine->get_record_mode() == 2 && 
				record_engine->current_position > record_engine->get_loop_duration())
			{
// loop
// must stop the thread to relocate
				result = file->write_video_buffer(buffer_position, 
							0, 
							record->use_floatingpoint(),
							(record->get_video_driver() == CAPTURE_LML ||
								record->get_video_driver() == CAPTURE_FIREWIRE));
				file->stop_video_thread();
				file->set_video_position(0, record->get_frame_rate());
				file->start_video_thread(buffer_size, 0, 2, 
							(record->get_video_driver() == CAPTURE_LML ||
							record->get_video_driver() == CAPTURE_FIREWIRE));
				buffer_position = buffer_size;
				record_start = -total_frames - 1;
				if(!record->do_audio) record_engine->update_position(0);
			}
			else
			if(result || 
				(record_engine->get_record_mode() == 1 && record_engine->current_position > record_engine->get_loop_duration()) ||
				single_frame)
			{
// Timed record and the timer has run out.
// Single frame record.
				done = 1;
// Signal the audio thread to stop since it is required for shared devices.
				if(record->do_audio) record_thread->record_audio->done = 1;
			}
		}

// Advance to next frame
		total_frames++;
	}

	if(result)
	{
		if(!record->do_audio && file)
		{
			ErrorBox error_box;
			error_box.create_objects("No space left on disk.");
			error_box.run_window();
		}
	}

	cleanup_recording();
}

int RecordVideo::unhang_thread()
{
printf("RecordVideo::unhang_thread\n");
	Thread::end();
return 0;
}

