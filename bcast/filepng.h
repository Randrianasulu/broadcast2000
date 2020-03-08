#ifndef FILEPNG_H
#define FILEPNG_H

#include <png.h>

#include "file.inc"
#include "filebase.h"
#include "vframe.inc"

class FilePNG : public FileBase
{
public:
	FilePNG(Asset *asset, File *file);
	~FilePNG();

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
// Since png files are usually only read once, we constitutively 
// use float and alpha for it.
	int read_raw();
	int reset_parameters_derived();
	int init_row_pointers(png_bytep *row_pointers, int row_bytes);
	int delete_row_pointers(png_bytep *row_pointers);
	int import_row_grey(VPixel *output, png_bytep row_pointer, int bit_depth);
	int import_row_grey_alpha(VPixel *output, png_bytep row_pointer, int bit_depth);
	int import_row_rgb(VPixel *output, png_bytep row_pointer, int bit_depth);
	int import_row_rgb_alpha(VPixel *output, png_bytep row_pointer, int bit_depth);
	int import_row_palette(VPixel *output, png_bytep row_pointer, int bit_depth, png_color *palette, int num_palette);

	FILE *stream;

	VFrame *data;

	long data_len;
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info;	
};


#endif
