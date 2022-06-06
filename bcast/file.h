#ifndef FILE_H
#define FILE_H

#include <stdio.h>

#include "arraylist.h"
#include "assets.inc"
#include "byteorder.h"
#include "file.inc"
#include "filebase.inc"
#include "filethread.inc"
#include "filehtal.inc"
#include "formatwindow.inc"
#include "mutex.h"
#include "pluginbuffer.inc"
#include "pluginioserver.inc"
#include "pluginserver.inc"
#include "sema.h"
#include "vframe.inc"


// ======================================= include file types here

// generic file opened by user
class File
{
public:
	File();
	~File();

// Get attributes for various file formats.
// The dither parameter is carried over from recording, where dither is done at the device.
	int get_audio_options(ArrayList<PluginServer*> *plugindb, Asset *asset, int *dither);
	int get_video_options(ArrayList<PluginServer*> *plugindb, Asset *asset, int recording);

// ===================================== start here
	int set_processors(int cpus);   // Set the number of cpus for certain codecs.
	int set_preload(long size);     // Set the number of bytes to preload during reads.

// return 0 if success/found format
// return 1 if failure/file not found
// return 2 if need format
// return 3 if project
// Format may be preset if the asset format is not 0.
	int try_to_open_file(ArrayList<PluginServer*> *plugindb, Asset *asset, int rd, int wr);

// start a thread for writing to avoid blocking during record
	int start_audio_thread(long buffer_size, int total_buffers = 2);
	int stop_audio_thread();
	int start_video_thread(long buffer_size, int color_model, int total_buffers, int compressed);
	int stop_video_thread();
	int lock_read();
	int unlock_read();

// write any headers and close file
	int close_file();

// set channel for buffer accesses
	int set_channel(int channel);

// set layer for video read
	int set_layer(int layer);

// set for dithering from 24 bits
	int set_dither();

// get length of file
	long get_length();
	long get_audio_length();
	long get_video_length(float base_framerate);

// seek to various points in the file
	int seek_end();
	int seek_start();

// get current position
	long get_position();
	long get_audio_position();
	long get_video_position(float base_framerate);

// get the number of bytes in buffers in this File object
	long get_memory_usage();

// set position in samples
	int set_position(long x);
	int set_audio_position(long x);
	int set_video_position(long x, float base_framerate);

// write samples for the current channel
// written to disk and file pointer updated after last channel is written
// return 1 if failed
// subsequent writes must be <= than first write's size because of buffers
	int write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len);

// Only called by filethread
	int write_frames(VFrame ***frames, 
			PluginBuffer *video_ram, 
			int len, 
			int use_alpha, 
			int use_float);

// For writing buffers in a background thread use these functions to get the buffer.
// Get a pointer to a buffer to write to.
	float** get_audio_buffer();
	VFrame*** get_video_buffer();

// Schedule a buffer for writing on the thread.
// thread calls write_samples
	int write_audio_buffer(long len);
	int write_video_buffer(long len, 
			int use_alpha, 
			int use_float, 
			int compressed = 0);

// Read samples for one channel into a shared memory segment.
// The offset is the offset in floats from the beginning of the buffer and the len
// is the length in floats from the offset.
// advances file pointer
// return 1 if failed
	int read_samples(PluginBuffer *buffer, long offset, long len);

// Overlay samples with start and end crossfade into a shared memory segment.
// The offset is the offset in floats from the beginning of the buffer and the len
// is the length in floats from the offset.
// For double speed playback the same amount of data is read but buffer is only half filled.
	int render_samples(PluginBuffer *buffer, long offset, long len,
		long lfeather_len, float lfeather_gain, float lfeather_slope);

// Return a pointer to the frame in a video file for drawing or 0.
// The following routine copies a frame once to a temporary buffer and either 
// return a pointer to the temporary buffer or copy the temporary buffer again.
	VFrame* read_frame(int use_alpha, int use_float);

// Overlay the frame on the user supplied input buffer for rendering.
// The offset is the offset in bytes from the beginning of the shmem buffer to the frame.
// We don't use pixel offsets because sometimes the frame is RGB24.
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);

// Single copy routine for direct playback.
// Read into the framebuffer directly
// The offset is the offset in bytes from the beginning of the shmem buffer to the frame.
	int read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset);

// The following involve no extra copies.
// Direct copy routines for direct copy playback
	int can_copy_from(Asset *asset, int output_w, int output_h); // This file can copy frames directly from the asset
	int get_render_strategy(ArrayList<int>* render_strategies);
	long compressed_frame_size();
	int read_compressed_frame(VFrame *buffer);
	int write_compressed_frame(VFrame *buffer);

// queries about file formats
	int supports_video(ArrayList<PluginServer*> *plugindb, char *format);   // returns 1 if the format supports video or audio
	int supports_audio(ArrayList<PluginServer*> *plugindb, char *format);
	int strtoformat(ArrayList<PluginServer*> *plugindb, char *format);
	char* formattostr(ArrayList<PluginServer*> *plugindb, int format);
	int strtobits(const char *bits);
	const char* bitstostr(int bits);
	static const char *strtocompression(const char *string);
	static const char *compressiontostr(const char *string);
	int bytes_per_sample(int bits); // Convert the bit descriptor into a byte count.

	Asset *asset;
	FileBase *file; // virtual class for file type
// Threads for writing data in the background.
	FileThread *audio_thread, *video_thread; 
// Lock writes while recording video and audio.
	Sema write_lock, read_lock;
	FormatAWindow *aformat_window;
	FormatVWindow *vformat_window;
	int cpus;
	long playback_preload;

private:
	PluginIOServer *aformat_plugin, *vformat_plugin;

	int getting_video_options;
	int getting_audio_options;
	Mutex aformat_completion, vformat_completion;
};

#endif
