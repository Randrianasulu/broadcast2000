#include <string.h>
#include "assets.h"
#include "file.h"
#include "filethread.h"
#include "pluginbuffer.h"
#include "vframe.h"

#include <unistd.h>

FileThread::FileThread(File *file, int do_audio, int do_video) : Thread()
{
	this->file = file;
	this->do_audio = do_audio;
	this->do_video = do_video;
	for(int i = 0; i < RING_BUFFERS; i++)
	{
		output_lock[i].lock();
		input_lock[i].unlock();
	}

	current_buffer = 0;
	return_value = 0;
	local_buffer = 0;
	video_ram = 0;
}

FileThread::~FileThread()
{
}

void FileThread::run()
{
	int done = 0;
	int i, j, result;

	while(!done)
	{
		output_lock[local_buffer].lock();
		return_value = 0;

		if(!last_buffer[local_buffer])
		{
			if(output_size[local_buffer])
			{
				if(do_audio)
				{
					result = file->write_samples(audio_buffer[local_buffer], 
						audio_ram, 
						byte_offset[local_buffer], 
						buffer_size,
						output_size[local_buffer]);
				}
				else
				if(do_video)
				{
					result = 0;
					if(is_compressed[local_buffer])
					{
						for(j = 0; j < file->asset->layers && !result; j++)
							for(i = 0; i < output_size[local_buffer] && !result; i++)
								result = file->write_compressed_frame(video_buffer[local_buffer][j][i]);
					}
					else
					{
						result = file->write_frames(video_buffer[local_buffer], 
							video_ram, 
							output_size[local_buffer],
							use_alpha, 
							use_float);
					}
				}

				return_value = result;
			}
			else
				return_value = 0;
		}
		else
			done = 1;

		input_lock[local_buffer].unlock();
		local_buffer++;
		if(local_buffer >= ring_buffers) local_buffer = 0;
	}
}



int FileThread::stop_writing()
{
	int i, buffer, layer, frame;
// signal thread to stop
	input_lock[current_buffer].lock();

	last_buffer[current_buffer] = 1;
	output_lock[current_buffer].unlock();

	swap_buffer();

// wait for thread to finish
	join();

// delete buffers
	if(do_audio)
	{
		delete audio_ram;

		for(buffer = 0; buffer < ring_buffers; buffer++)
		{
			delete [] audio_buffer[buffer];
		}
	}

	if(do_video)
	{
		if(video_ram) delete video_ram;
		for(buffer = 0; buffer > ring_buffers; buffer++)
		{
			for(layer = 0; layer < file->asset->layers; layer++)
			{
				for(frame = 0; frame < buffer_size; frame++)
				{
					delete video_buffer[buffer][layer][frame];
				}
				delete [] video_buffer[buffer][layer];
			}
			delete [] video_buffer[buffer];
		}
	}
	return 0;
return 0;
}

int FileThread::start_writing(long buffer_size, 
		int color_model, 
		int ring_buffers, 
		int compressed)
{
// allocate buffers
	int buffer, layer, frame;
	long offset, bytes_per_frame;
	long total_size;

	this->ring_buffers = ring_buffers;
	this->buffer_size = buffer_size;

	if(do_audio)
	{
		audio_ram = new PluginBuffer(ring_buffers * 
				file->asset->channels * 
				buffer_size * 
				sizeof(float), 1);
		offset = 0;

		for(buffer = 0; buffer < ring_buffers; buffer++)
		{
			audio_buffer[buffer] = new float*[file->asset->channels];
			byte_offset[buffer] = offset;

			for(int channel = 0; channel < file->asset->channels; channel++)
			{
				audio_buffer[buffer][channel] = (float*)((char*)audio_ram->get_data() + offset);
				offset += buffer_size * sizeof(float);
			}
		}
	}

	if(do_video)
	{
		this->color_model = color_model;
		offset = 0;
		bytes_per_frame = file->asset->width * 
			file->asset->height * 
			VFrame::bytes_per_pixel(color_model) + 4;

		if(!compressed)
			video_ram = new PluginBuffer(ring_buffers * 
					file->asset->layers * 
					buffer_size *
					bytes_per_frame, 1);

		for(buffer = 0; buffer < ring_buffers; buffer++)
		{
			video_buffer[buffer] = new VFrame**[file->asset->layers];
			for(layer = 0; layer < file->asset->layers; layer++)
			{
				video_buffer[buffer][layer] = new VFrame*[buffer_size];
				for(frame = 0; frame < buffer_size; frame++)
				{
					if(compressed)
						video_buffer[buffer][layer][frame] = new VFrame;
					else
					{
						video_buffer[buffer][layer][frame] = 
							new VFrame((unsigned char*)video_ram->get_data() + offset, 
								file->asset->width, 
								file->asset->height, 
								color_model);
						video_buffer[buffer][layer][frame]->set_shm_offset(offset);
						offset += bytes_per_frame;
					}
				}
			}
		}
	}

	for(int i = 0; i < ring_buffers; i++)
	{
		last_buffer[i] = 0;
	}

	synchronous = 1;

	start();
	return 0;
return 0;
}

float** FileThread::get_audio_buffer()
{
	input_lock[current_buffer].lock();
	return audio_buffer[current_buffer];
}

VFrame*** FileThread::get_video_buffer()
{
	input_lock[current_buffer].lock();
	return video_buffer[current_buffer];
}

int FileThread::write_buffer(long size, int use_alpha, int use_float, int compressed)
{
	this->use_alpha = use_alpha;
	this->use_float = use_float;
	output_size[current_buffer] = size;
	is_compressed[current_buffer] = compressed;

// unlock the output lock
	output_lock[current_buffer].unlock();

	swap_buffer();

	return return_value;
return 0;
}

int FileThread::swap_buffer()
{
	current_buffer++;
	if(current_buffer >= ring_buffers) current_buffer = 0;
return 0;
}
