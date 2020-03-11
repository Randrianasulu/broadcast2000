#include <string.h>
#include "assets.h"
#include "audiodevice.h"
#include "file.h"
#include "pluginbuffer.h"
#include "previewaudio.h"
#include "record.h"
#include "recordgui.h"
#include "recordengine.h"

PreviewAudio::PreviewAudio(Record *record, 
	RecordEngine *record_engine, 
	RecordPreview *preview_thread)
 : Thread()
{
	this->record = record;
	this->record_engine = record_engine;
	this->preview_thread = preview_thread;
	fragments = 10;

	output[0] = 0;
	peak_history = 0;
	peak_sample = 0;
}

PreviewAudio::~PreviewAudio()
{
}


int PreviewAudio::start_preview(long position, File *file)
{
	int i, j;

	meter = record_engine->gui->meter;
	device = record_engine->adevice;
	fragment_size = record->get_samplerate() / fragments;
	for(buffer_size = fragment_size; 
		buffer_size < record->get_out_buffersize(); 
		buffer_size += fragment_size)
		;

	done = 0;

	this->file = file;
	this->current_position = position;
	this->preview_start = position;

	playback_channels = file->asset->channels;
	preview_end = file->get_length();

// initialize meters	
	total_peaks = 2 * buffer_size / fragment_size + 256000 / fragment_size;
	peak_history = new float*[playback_channels];
	for(i = 0, j = 0; i < playback_channels; i++)
	{
		peak_history[i] = new float[total_peaks];
		for(j = 0; j < total_peaks; j++)
		{
			peak_history[i][j] = 0;
		}
		meter[i]->set_delays(record_engine->get_meter_over_hold(buffer_size / fragment_size), 
			record_engine->get_meter_peak_hold(buffer_size / fragment_size));
	}
	peak_sample = new long[total_peaks];
	for(i = 0; i < total_peaks; i++)
	{
		peak_sample[i] = -1;
	}
	current_peak = 0;

	arm_buffer();
	device->start_playback();
	Thread::synchronous = 1;
	Thread::start();
	return 0;
return 0;
}

int PreviewAudio::stop_preview()
{
	last_position = device->current_position() + preview_start;
	done = 1;
	device->interrupt_playback();
return 0;
}

int PreviewAudio::arm_buffer()
{
	int i, j, fragment_end;
	int k;
	float min, max, *current_buffer;
	int last_buffer = 0;

	if(preview_end - current_position <= buffer_size)
	{
		buffer_size = preview_end - current_position;
		last_buffer = 1;
	}

	if(!output[0]) 
	{
// Allocate playback buffers
		for(i = 0; i < playback_channels; i++) 
		{
			shared_output[i] = new PluginBuffer(buffer_size, sizeof(float));
			output[i] = (float*)shared_output[i]->get_data();
//			output[i] = new float[buffer_size];
		}
	}

	for(i = 0; i < playback_channels; i++)
	{
		current_buffer = output[i];
		file->lock_read();
		file->set_position(current_position);
		file->set_channel(i);
		file->read_samples(shared_output[i], 0, buffer_size);
//		file->read_samples(current_buffer, buffer_size);
		file->unlock_read();
	}


// get peaks
	for(i = 0; i < buffer_size; )
	{
		fragment_end = i + fragment_size;
		if(fragment_end > buffer_size) fragment_end = buffer_size;

		min = max = 0;
		for(j = 0; j < playback_channels; j++)
		{
			current_buffer = output[j];

			for(k = i; k < fragment_end; k++)
			{
				if(current_buffer[k] > max) max = current_buffer[k];
				else
				if(current_buffer[k] < min) min = current_buffer[k];
			}

			if(fabs(max) > fabs(min)) 
				peak_history[j][current_peak] = fabs(max);
			else
				peak_history[j][current_peak] = fabs(min);

			peak_sample[current_peak] = current_position + fragment_end;
		}
			
		i = fragment_end;
		current_peak++;
		if(current_peak == total_peaks) current_peak = 0;
	}
	
	device->write_buffer(output, buffer_size, playback_channels);
	if(last_buffer) device->set_last_buffer();
	current_position += buffer_size;
return 0;
}

void PreviewAudio::run()
{
	PreviewPosition counter(this);
	output_peak = 0;
	last_peak = 0;
	counter.synchronous = 1;
	counter.start();

	while(!done && current_position < preview_end)
	{
		arm_buffer();
	}

	done = 1;
	device->wait_for_completion();

	counter.join();

	record_engine->update_position(current_position);
	if(output[0])
	{
		for(int i = 0; i < playback_channels; i++) delete shared_output[i];     // delete the objects
	}
	output[0] = 0;
}

int PreviewAudio::update_current_position()
{
	static long new_position, pass, starting_peak, i;
	new_position = get_current_position();

// get peak
	pass = 0;
	starting_peak = output_peak;
	while(peak_sample[output_peak] < new_position && pass == 0)
	{
		output_peak++;
		if(output_peak == total_peaks) output_peak = 0;
		if(output_peak == starting_peak) pass++;
	}
	if(peak_sample[output_peak] > new_position)
	{
		output_peak--;
		if(output_peak < 0) output_peak = total_peaks - 1;
	}
	
	if(last_peak != peak_sample[output_peak])
	{
		last_peak = peak_sample[output_peak];
		
		record_engine->lock_window();
		for(i = 0; i < playback_channels; i++)
		{
			meter[i]->update(peak_history[i][output_peak], peak_history[i][output_peak] > 1);
		}
		record_engine->unlock_window();
	}
	
	if(current_position > preview_end) new_position = preview_end;
	record_engine->update_position(new_position);
return 0;
}

long PreviewAudio::get_current_position()
{
	if(!done)
		return device->current_position() + preview_start;
	else
		return -1;
}

long PreviewAudio::absolute_position()
{
	if(!done)
		return device->current_position();
	else
		return -1;
}





PreviewPosition::PreviewPosition(PreviewAudio *thread)
 : Thread()
{ this->thread = thread; }

PreviewPosition::~PreviewPosition() {}

void PreviewPosition::run()
{
	struct timeval delay_duration;

	while(!thread->done)
	{
		thread->update_current_position();
// select resets delay_duration
		delay_duration.tv_sec = 0;
		delay_duration.tv_usec  = 1000000 / thread->fragments;
		select(0,  NULL,  NULL, NULL, &delay_duration);
	}
}
