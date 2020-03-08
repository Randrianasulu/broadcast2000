#ifndef FILEMOV_H
#define FILEMOV_H

#include "file.inc"
#include "filebase.h"
#include "libmjpeg.h"
#include "mutex.h"
#include "thread.h"

// ./Quicktime
#include "jpeg_old.h"
#include "quicktime.h"

class FileMOVThread;

class ThreadStruct
{
public:
	ThreadStruct();
	~ThreadStruct();

	void load_output(mjpeg_t *mjpeg);

	VFrame *input;
	unsigned char *output;  // Output buffer
	long output_size;        // Size of output buffer
	long output_allocated;  // Allocation of output buffer
	Mutex completion_lock;
};

class FileMOV : public FileBase
{
public:
	FileMOV(Asset *asset, File *file);
	~FileMOV();
	
	friend FileMOVThread;

	int open_file(int rd, int wr);
	int close_file_derived();
	int read_header();
	long get_video_length();
	long get_audio_length();
	long get_position();
	long get_video_position();
	long get_audio_position();
	long get_memory_usage();
	int set_video_position(long x);
	int set_audio_position(long x);
	int set_channel(int channel);  // set audio channel for reading
	int set_layer(int layer);      // set layer for reading
	int seek_end();
	int seek_start();
	int write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len);
	int write_frames(VFrame ***frames, 
			PluginBuffer *video_ram, 
			int len, 
			int use_alpha, 
			int use_float);
	long compressed_frame_size();
	int read_compressed_frame(VFrame *buffer);
	int write_compressed_frame(VFrame *buffer);
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	VFrame* read_frame(int use_alpha, int use_float);
	int read_samples(PluginBuffer *shared_buffer, long offset, long len, 
		int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope);
	int read_raw_frame_possible();
	int read_raw_frame(VFrame *buffer, PluginBuffer *plugin_buffer, long byte_offset);

// Direct copy routines
	int can_copy_from(Asset *asset); // This file can copy frames directly from the asset
	int get_render_strategy(ArrayList<int>* render_strategies);

private:
// Test for 32bit overflow and reopen
	int test_length(int result);
// Increment path and open a new file when 32bits are exceeded.
	int reopen_file();
// read raw audio data
	int read_raw(char *buffer, long samples, int track);  
// overlay raw frame from the current layer and position
	int read_raw(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	int reset_parameters_derived();
	int quicktime_atracks;
	int quicktime_vtracks;
// current positions for when the file descriptor doesn't have the right position
	quicktime_t *file;
	VFrame *data; // frame to return through read_frame
	int use_bcast_audio;    // Don't use the library for audio
	int depth;        // Depth in bits per pixel
	long frames_correction;  // Correction after 32bit overflow
	long samples_correction;  // Correction after 32bit overflow

// An array of frames for threads to look up and compress on their own.
	ArrayList<ThreadStruct*> threadframes;

	int total_threadframes;     // Number of thread frames in this buffer
	int current_threadframe;    // Next threadframe to compress
	Mutex threadframe_lock;     // Lock threadframe array.

	FileMOVThread **threads;   // One thread for every CPU
	char prefix_path[1024];    // Prefix for new file when 2G limit is exceeded
	int suffix_number;         // Number for new file
};


// Encoder thread to parallelize certain compression formats, mainly JPEG.
// Only works when no alpha.

class FileMOVThread : public Thread
{
public:
	FileMOVThread(FileMOV *filemov, int fields);
	~FileMOVThread();

	int start_encoding();
	int stop_encoding();
	int encode_buffer();
	void run();

	ThreadStruct *threadframe;    // The frame currently being processed.
	int done;
	FileMOV *filemov;
	Mutex input_lock;     // Wait for new array of threads or completion.
	Mutex quicktime_lock;  // Lock out reopen_file
	mjpeg_t *mjpeg;
	int fields;
};



#endif
