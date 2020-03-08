#include <string.h>
#include "apluginarray.h"
#include "atrack.h"
#include "file.h"
#include "mainwindow.h"
#include "playbackengine.h"
#include "pluginbuffer.h"
#include "pluginserver.h"
#include "preferences.h"
#include "recordableatracks.h"


APluginArray::APluginArray(MainWindow *mwindow)
 : PluginArray(mwindow)
{
	tracks = new RecordableATracks(mwindow->tracks, mwindow->patches);
}

APluginArray::~APluginArray()
{
	delete tracks;
}

long APluginArray::get_bufsize()
{
	//return mwindow->playback_engine->audio_read_length;
	return mwindow->sample_rate;
}

PluginBuffer* APluginArray::get_buffer()
{
	return new PluginBuffer(get_bufsize(), sizeof(float));
}

int APluginArray::total_tracks()
{
	return tracks->total;
return 0;
}

Track* APluginArray::track_number(int number)
{
	return (Track*)tracks->values[number];
}


int APluginArray::start_plugins_derived()
{
	int current_plugin, current_buffer, j;
	buffer = new float*[total_tracks()];
	shared_buffer = new PluginBuffer*[total_tracks()];

	for(current_plugin = 0, current_buffer = 0; current_plugin < total; current_plugin++)
	{
		values[current_plugin]->start_plugin();

		for(j = 0; j < values[current_plugin]->total_in_buffers; j++)
		{
			buffer[current_buffer] = 
				(float*)values[current_plugin]->data_in.values[j]->get_data();
			shared_buffer[current_buffer] = 
				values[current_plugin]->data_in.values[j];
			current_buffer++;
		}
	}

// Get file buffers for output
	file->start_audio_thread(values[0]->in_buffer_size, mwindow->preferences->smp ? 2 : 1);
return 0;
}


int APluginArray::start_realtime_plugins_derived()
{
	int i;
	buffer = new float*[total_tracks()];
	shared_buffer = new PluginBuffer*[total_tracks()];

	for(i = 0; i < total_tracks(); i++)
	{
		buffer[i] = (float*)realtime_buffers[i]->get_data();
		shared_buffer[i] = realtime_buffers[i];
	}

// Get file buffers for output
	file->start_audio_thread(values[0]->in_buffer_size);
return 0;
}

int APluginArray::write_samples_derived(long samples_written)
{
// test for overload
	int i, j;

	output_buffer = file->get_audio_buffer();
	for(i = 0; i < total_tracks(); i++)
	{
		for(j = 0; j < samples_written; j++)
		{
			if(buffer[i][j] > 1) output_buffer[i][j] = 1;
			else
			if(buffer[i][j] < -1) output_buffer[i][j] = -1;
			else
			output_buffer[i][j] = buffer[i][j];
		}
	}

	return file->write_audio_buffer(samples_written);
return 0;
}

int APluginArray::stop_plugins_derived()
{
	delete [] buffer;
	delete [] shared_buffer;
	file->stop_audio_thread();
return 0;
}

int APluginArray::render_track(int track, long fragment_len, long position)
{
	return ((ATrack*)track_number(track))->render(shared_buffer[track], 
				0, 
				fragment_len, 
				position);
//	return ((ATrack*)track_number(track))->render(buffer[track], fragment_len, position);
return 0;
}
