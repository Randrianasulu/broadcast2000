#include <string.h>
#include "assets.h"
#include "indexthread.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "pluginbuffer.inc"
#include "preferences.h"
#include "tracks.h"



IndexThread::IndexThread(MainWindow *mwindow, 
						Asset *asset,
						char *index_filename,
						long buffer_size, 
						long length_source)
{
	this->asset = asset;
	this->buffer_size = buffer_size;
	this->length_source = length_source;
	this->mwindow = mwindow;
	this->index_filename = index_filename;
	
// initialize output data
	long index_size = mwindow->preferences->index_size / sizeof(float) + 1;      // size of output file in floats
	if(asset->index_buffer) delete asset->index_buffer;
	if(asset->index_offsets) delete asset->index_offsets;
	asset->index_buffer = new float[index_size];  // buffer used for drawing during the build
	asset->index_offsets = new long[asset->channels];
	for(int i = 0; i < index_size; i++) asset->index_buffer[i] = 0;
// initialization is completed in run

	for(int i = 0; i < TOTAL_BUFFERS; i++)
	{
		shared_buffer_in[i] = new PluginBuffer*[asset->channels];
		buffer_in[i] = new float*[asset->channels];
		for(int j = 0; j < asset->channels; j++)
		{
			shared_buffer_in[i][j] = new PluginBuffer(buffer_size, sizeof(float));
			buffer_in[i][j] = (float*)shared_buffer_in[i][j]->get_data();
		}
		output_lock[i].lock();
	}
	
	interrupt_flag = 0;
}

IndexThread::~IndexThread()
{
	for(int i = 0; i < TOTAL_BUFFERS; i++)
	{
		for(int j = 0; j < asset->channels; j++)
		{
			delete shared_buffer_in[i][j];
		}
		delete shared_buffer_in[i];
	}
	
	delete asset->index_buffer;
	asset->index_buffer = 0;
}

int IndexThread::start_build()
{
	synchronous = 1;
	interrupt_flag = 0;
	current_buffer = 0;
	for(int i = 0; i <  TOTAL_BUFFERS; i++) last_buffer[i] = 0;
	start();
return 0;
}

int IndexThread::stop_build()
{
	join();
return 0;
}

void IndexThread::run()
{
	int done = 0;

// current high samples in index
	long *highpoint;            
// current low samples in the index
	long *lowpoint;             
// position in current indexframe
	long *frame_position;

	highpoint = new long[asset->channels];            
// current low samples in the index
	lowpoint = new long[asset->channels];             
// position in current indexframe
	frame_position = new long[asset->channels];

// predict first highpoint for each channel plus padding and initialize it
	for(long channel = 0; channel < asset->channels; channel++)
	{
		highpoint[channel] = asset->index_offsets[channel] = (length_source / asset->index_zoom * 2 + 1) * channel;
		lowpoint[channel] = highpoint[channel] + 1;

// zero the first point
		asset->index_buffer[highpoint[channel]] = 0;
		asset->index_buffer[lowpoint[channel]] = 0;
		frame_position[channel] = 0;
	}

	long index_start = 0;    // end of index during last edit update
	asset->index_end = 0;      // samples in source completed
	asset->old_index_end = 0;
	asset->index_status = 2;
	long zoomx = asset->index_zoom;
	float *index_buffer = asset->index_buffer;    // output of index build

	redraw_edits(1);

	while(!interrupt_flag && !done)
	{
		output_lock[current_buffer].lock();
		
		if(last_buffer[current_buffer]) done = 1;
		if(!interrupt_flag && !done)
		{
// process buffer
			long fragment_size = input_len[current_buffer];

			for(int channel = 0; channel < asset->channels; channel++)
			{
				long *highpoint_channel = &highpoint[channel];
				long *lowpoint_channel = &lowpoint[channel];
				long *frame_position_channel = &frame_position[channel];
				float *buffer_source = buffer_in[current_buffer][channel];

				for(long i = 0; i < fragment_size; i++)
				{
					if(*frame_position_channel == zoomx)
					{
						*highpoint_channel += 2;
						*lowpoint_channel += 2;
						*frame_position_channel = 0;
// store and reset output values
						index_buffer[*highpoint_channel] = index_buffer[*lowpoint_channel] = buffer_source[i];
					}
					else
					{
// get high and low points
						if(buffer_source[i] > index_buffer[*highpoint_channel]) index_buffer[*highpoint_channel] = buffer_source[i];
						else if(buffer_source[i] < index_buffer[*lowpoint_channel]) index_buffer[*lowpoint_channel] = buffer_source[i];
					}
					(*frame_position_channel)++;
				} // end index one buffer
			}

			asset->index_end += fragment_size;

// draw simultaneously with build
			if(asset->index_end - index_start > mwindow->zoom_sample * 10 || asset->index_end >= length_source)
			{
				redraw_edits(1);
				index_start = asset->index_end;
			}
		}

		input_lock[current_buffer].unlock();
		current_buffer++;
		if(current_buffer >= TOTAL_BUFFERS) current_buffer = 0;
	}

// ================================== write the index file to disk
	FILE *file;
	if(!(file = fopen(index_filename, "wb")))
	{
// failed to create it
		printf("IndexThread::run() Couldn't write index file %s to disk.\n", index_filename);
	}
	else
	{
		fwrite((char*)&(asset->index_start), sizeof(long), 1, file);
		
		asset->index_status = 0;
		asset->write(mwindow, file, 1, 0);
		asset->index_start = ftell(file);
		fseek(file, 0, SEEK_SET);
		fwrite((char*)&(asset->index_start), sizeof(long), 1, file);
		fseek(file, asset->index_start, SEEK_SET);
		
		fwrite(asset->index_buffer, (lowpoint[asset->channels - 1] + 1) * sizeof(float), 1, file);
		fclose(file);
	}

// done
	delete [] highpoint;            
// current low samples in the index
	delete [] lowpoint;             
// position in current indexframe
	delete [] frame_position;
	asset->index_status = 1;      // force reread of index
	asset->index_end = length_source;
	asset->old_index_end = 0;
	asset->index_start = 0;
// correct errors in drawing
	//redraw_edits(1);
}

int IndexThread::redraw_edits(int flash)
{
	mwindow->gui->lock_window();
	mwindow->tracks->set_index_file(flash, asset);
	mwindow->gui->unlock_window();
	asset->old_index_end = asset->index_end;
	return 0;
return 0;
}
