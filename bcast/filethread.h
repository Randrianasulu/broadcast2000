#ifndef FILETHREAD_H
#define FILETHREAD_H

#include "file.inc"
#include "mutex.h"
#include "thread.h"
#include "pluginbuffer.inc"
#include "vframe.inc"

#define RING_BUFFERS 2

class FileThread : public Thread
{
public:
	FileThread(File *file, int do_audio, int do_video);
	~FileThread();

// write data into next available buffer
	int write_buffer(long size, int use_alpha = 0, int use_float = 0, int compressed = 0);
	float** get_audio_buffer();     // get pointer to next buffer to be written and lock it
	VFrame*** get_video_buffer();     // get pointer to next frame to be written and lock it

	void run();
	int start_writing(long buffer_size, 
			int color_model, 
			int ring_buffers, 
			int compressed);
	int stop_writing();
	int swap_buffer();

	float **audio_buffer[RING_BUFFERS];
	long byte_offset[RING_BUFFERS];    // Offsets of audio buffers [ring buffer]
	VFrame ***video_buffer[RING_BUFFERS];      // [ring buffer][track][frame array][frame]
	PluginBuffer *video_ram, *audio_ram;
	long output_size[RING_BUFFERS];  // Number of frames or samples to write
	int is_compressed[RING_BUFFERS]; // Whether to use the compressed data in the frame
	Mutex output_lock[RING_BUFFERS], input_lock[RING_BUFFERS];
	int current_buffer;
	int local_buffer;
	int last_buffer[RING_BUFFERS];
	int return_value;
	int do_audio;
	int do_video;
	int use_alpha, use_float;
	File *file;
	int ring_buffers;
	int buffer_size;    // Frames or samples per ring buffer
// Color model of frames
	int color_model;
};



#endif
