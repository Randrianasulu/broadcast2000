#include <string.h>
#include "file.h"
#include "mainwindow.h"
#include "pluginbuffer.h"
#include "pluginserver.h"
#include "preferences.h"
#include "recordablevtracks.h"
#include "vframe.h"
#include "vpluginarray.h"
#include "vtrack.h"

VPluginArray::VPluginArray(MainWindow *mwindow)
 : PluginArray(mwindow)
{
	tracks = new RecordableVTracks(mwindow->tracks, mwindow->patches);
}

VPluginArray::~VPluginArray()
{
	delete tracks;
}

long VPluginArray::get_bufsize()
{
	return 1;
}

PluginBuffer* VPluginArray::get_buffer()
{
	return new PluginBuffer(get_bufsize() * mwindow->track_w * mwindow->track_h + 1, sizeof(VPixel));
}

int VPluginArray::total_tracks()
{
	return tracks->total;
return 0;
}

Track* VPluginArray::track_number(int number)
{
	return (Track*)tracks->values[number];
}


int VPluginArray::start_plugins_derived()
{
	long current_plugin, current_buffer, j, k;
	buffer = new VFrame**[total_tracks()];
	shared_buffer = new PluginBuffer**[total_tracks()];

// Generate pointers to the shared memory owned by plugins
	for(current_plugin = 0, current_buffer = 0; current_plugin < total; current_plugin++)
	{
		values[current_plugin]->start_plugin();

		for(j = 0; j < values[current_plugin]->total_in_buffers; j++)
		{
			buffer[current_buffer] = new VFrame*[get_bufsize()];
			shared_buffer[current_buffer] = new PluginBuffer*[get_bufsize()];

			for(k = 0; k < get_bufsize(); k++)
			{
				buffer[current_buffer][k] = new VFrame(
					(unsigned char*)((VPixel*)values[current_plugin]->data_in.values[j]->get_data() + 
						k * mwindow->track_w * mwindow->track_h), 
					mwindow->track_w, mwindow->track_h);
				shared_buffer[current_buffer][j] = values[current_plugin]->data_in.values[j];
			}
			current_buffer++;
		}
	}

// Get file buffers for output
// Specify only 2 ring buffers for smp.
	file->start_video_thread(get_bufsize(), 1, mwindow->preferences->smp ? 2 : 1, 0);
return 0;
}

int VPluginArray::start_realtime_plugins_derived()
{
	int i, j;
	buffer = new VFrame**[total_tracks()];
	shared_buffer = new PluginBuffer**[total_tracks()];

// Generate pointers to the shared memory owned by plugins
	for(i = 0; i < total_tracks(); i++)
	{
		buffer[i] = new VFrame*[get_bufsize()];
		shared_buffer[i] = new PluginBuffer*[get_bufsize()];

		for(j = 0; j < get_bufsize(); j++)
		{
			buffer[i][j] = new VFrame(
				(unsigned char*)((VPixel*)realtime_buffers[i]->get_data() + 
					j * mwindow->track_w * mwindow->track_h), 
				mwindow->track_w, mwindow->track_h);
			shared_buffer[i][j] = realtime_buffers[i];
		}
	}

// Get file buffers for output
	file->start_video_thread(get_bufsize(), 1, 2, 0);
return 0;
}

int VPluginArray::write_frames_derived(long frames_written)
{
	int i, j;
	video_output = file->get_video_buffer();

// Copy to the file buffer
	for(i = 0; i < total_tracks(); i++)
	{
		for(j = 0; j < frames_written; j++)
		{
			video_output[i][j]->copy_from(buffer[i][j]);
		}
	}

	return file->write_video_buffer(frames_written, mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint);
return 0;
}

int VPluginArray::write_samples_derived(long frames_written)
{
	return write_frames_derived(frames_written);
return 0;
}

int VPluginArray::stop_plugins_derived()
{
	int i, j;
	for(i = 0; i < total_tracks(); i++)
	{
		for(j = 0; j < get_bufsize(); j++)
		{
			delete buffer[i][j];
		}
		delete buffer[i];
		delete shared_buffer[i];
	}
	delete buffer;
	delete shared_buffer;

	file->stop_video_thread();
return 0;
}


int VPluginArray::render_track(int track, long fragment_len, long position)
{
	return ((VTrack*)track_number(track))->render(buffer[track], 
					shared_buffer[track][0], 
					0, 
					fragment_len, 
					position, 
					1);
return 0;
}
