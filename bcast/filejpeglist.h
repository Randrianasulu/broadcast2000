#ifndef FILEJPEGLIST_H
#define FILEJPEGLIST_H

#include "file.inc"
#include "filebase.h"
#include "jpegwrapper.h"
#include "vframe.inc"

class FileJPEGList : public FileBase
{
public:
	FileJPEGList(Asset *asset, File *file);
	~FileJPEGList();

// basic commands for every file interpreter
	int open_file(int rd, int wr);
	int close_file_derived();
	long get_video_length();
	long get_memory_usage();
	int set_layer(int layer);
	int set_video_position(long x);
	int seek_start();
	int seek_end();
	long compressed_frame_size(); // Return size of frame at current position
	int read_compressed_frame(VFrame *buffer);
	int write_compressed_frame(VFrame *buffer);

// Direct copy routines
	int can_copy_from(Asset *asset); // This file can copy frames directly from the asset

	int read_header();
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
			float in_x1, float in_y1, float in_x2, float in_y2,
			float out_x1, float out_y1, float out_x2, float out_y2, 
			int alpha, int use_alpha, int use_float, int interpolate);
	VFrame* read_frame(int use_alpha, int use_float);

	int write_frames(VFrame ***frames, 
			PluginBuffer *video_ram, 
			int len, 
			int use_alpha, 
			int use_float);

private:
	char* create_path();
	int read_raw(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	int reset_parameters_derived();
	int write_header();
	ArrayList<char*> path_list;     // List of JPEG files in the case of a list
	VFrame *data;
	FILE *stream;
};


#endif
