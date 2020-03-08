#ifndef FILEJPEG_H
#define FILEJPEG_H

#include "file.inc"
#include "filebase.h"
#include "jpegwrapper.h"
#include "vframe.inc"

class FileJPEG : public FileBase
{
public:
	FileJPEG(Asset *asset, File *file);
	~FileJPEG();

// basic commands for every file interpreter
	int open_file(int rd, int wr);
	int close_file_derived();
	long get_video_length();
	long get_memory_usage();

	int read_header();
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	VFrame* read_frame(int use_alpha, int use_float);

private:
	int read_raw(int use_alpha, int use_float);
	int reset_parameters_derived();

// routines for all image files
	VFrame *data;
};


#endif
