#include <string.h>
#include "assets.h"
#include "audiodevice.h"
#include "errorbox.h"
#include "file.h"
#include "record.h"
#include "recordaudio.h"
#include "recordgui.h"
#include "recordengine.h"
#include "renderengine.h"


RecordAudio::RecordAudio(Record *record, 
				RecordEngine *record_engine, 
				RecordThread *record_thread, 
				AudioDevice *device)
 : Thread()
{
	this->record = record;
	this->record_engine = record_engine;
	this->record_thread = record_thread; 
	this->device = device;
	fragment_position = 0;
	total_samples = 0;
}

RecordAudio::~RecordAudio()
{
}

int RecordAudio::start_recording(File *file)
{
	this->file = file;
	fragment_size = record_engine->get_in_length();
	for(buffer_size = fragment_size; 
		buffer_size < record->get_out_length(); 
		buffer_size += fragment_size)
		;

	gui = record_engine->gui;
	meter = gui->meter;
	record_channels = record_engine->asset->channels;
	device->set_record_dither(record_engine->get_dither());
	if(record->get_realtime()) Thread::set_realtime();

	fragment_position = 0;
	timer.update();
	total_samples = 0;
	over = new int[record_channels];
	max = new float[record_channels];
	done = 0;
	Thread::synchronous = 1;
	Thread::start();
return 0;
}

int RecordAudio::stop_recording()
{
	done = 1;
	device->interrupt_crash();
	//Thread::join();
	return 0;
return 0;
}

void RecordAudio::run()
{
	int channel, buffer;
	int result = 0;
	Timer delayer;
	int total_clipped_samples = 0;
	int clipped_sample = 0;

	total_samples = 0;

// thread out I/O
	if(file)
	{
// Get a buffer from the file to record into.
		file->start_audio_thread(buffer_size);
		input = file->get_audio_buffer();
	}
	else
	{
// Make up a fake buffer.
		input = new float*[record_channels];

		for(int i = 0; i < record_channels; i++)
		{
			input[i] = new float[buffer_size];
		}
	}

	gui->total_clipped_samples = 0;
	gui->update_clipped_samples(0);

	while(!done && !result)
	{
// Handle data from the audio device.
		if(record->monitor_audio || file)
		{
			if(file)
			{
// Read into file's buffer for recording.
// device needs to write buffer starting at fragment position
				device->read_buffer(input, fragment_size, record_channels, over, max, fragment_position);
			}
			else
			{
// Read into monitor buffer for monitoring.
				device->read_buffer(input, fragment_size, record_channels, over, max, 0);
			}

// Update timer for synchronization
			timer_lock.lock();
			total_samples += fragment_size;
			timer.update();
			timer_lock.unlock();

// Update meters if monitoring
			if(record->monitor_audio && !done && !result)
			{
				gui->lock_window();
				for(channel = 0; channel < record_channels; channel++)
				{
					meter[channel]->update(max[channel], over[channel]);
				}

				gui->unlock_window();
			}

			if(record->monitor_audio || file)
			{
				clipped_sample = 0;
				for(channel = 0; channel < record_channels; channel++)
				{
					if(over[channel]) clipped_sample = 1;
				}
			}

// Write file if writing
			if(file)
			{
				fragment_position += fragment_size;

				if(fragment_position >= buffer_size)
				{
					result = write_buffer();
				}

				record_engine->update_position(get_position() + fragment_size);
				if(clipped_sample) gui->update_clipped_samples(++total_clipped_samples);

// handle different recording modes
				if(!result && 
					!done &&
					record_engine->get_record_mode() == 2 && 
					record_engine->current_position > record_engine->get_loop_duration())
				{
// loop
					set_position(0);
				}
				else
				if(result || 
					(record_engine->get_record_mode() == 1 && 
						record_engine->current_position > record_engine->get_loop_duration() &&
						!record->do_video))
				{
// timed record and the timer has run out or write error
					done = 1;
				}
			}
		}
		else
		{
// Don't handle audio data.
			delayer.delay(250);
			timer_lock.lock();
			total_samples += record->get_samplerate() / 4;
			timer.update();
			timer_lock.unlock();
		}
	}

	if(result)
	{
		ErrorBox error_box;
		error_box.create_objects("No space left on disk.");
		error_box.run_window();
	}

	if(file)
	{
// write last buffer
		result = file->write_audio_buffer(fragment_position);

// stop file I/O
		file->stop_audio_thread();
	}
	else
	{
// Delete the fake buffer.
		for(int i = 0; i < record_channels; i++)
		{
			meter[i]->reset();
			delete input[i];
		}
		delete input;
		input = 0;
	}

// reset meter
	gui->lock_window();
	for(channel = 0; channel < record_channels; channel++)
	{
		meter[channel]->reset();
	}

	gui->unlock_window();
	delete max;
	delete over;
}

int RecordAudio::write_buffer()
{
// block until buffer is ready for writing
	int result = file->write_audio_buffer(fragment_position);
	fragment_position = 0;
	input = file->get_audio_buffer();
	return result;
return 0;
}

int RecordAudio::set_position(long position)
{
	int result = file->write_audio_buffer(fragment_position);
	fragment_position = 0;

// must stop the thread to relocate
	file->stop_audio_thread();
	file->set_audio_position(position);
	record_engine->update_position(0);
	
	file->start_audio_thread(buffer_size);
	
	input = file->get_audio_buffer();
	return result;	
return 0;
}

long RecordAudio::get_position()
{
	return record_engine->current_position;
}

long RecordAudio::absolute_position()
{
	long result;
	if(!done)
	{
		timer_lock.lock();
		result = total_samples;
		result += (long)((float)timer.get_difference() / 1000 * record->get_samplerate());
		timer_lock.unlock();
		return result;
	}
	else
		return -1;
}

