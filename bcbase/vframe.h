#ifndef VCONFIG_H
#define VCONFIG_H

#include "vframe.inc"

class VFrame
{
public:
// Create new frame with shared data if *data is nonzero.
// Pass 0 to *data if private data is desired.
	VFrame(unsigned char *data, 
		int w, 
		int h, 
		int color_model = VFRAME_VPIXEL, 
		long bytes_per_line = -1);
// Create new frame for compressed data.
// Can't share data because the data is dynamically allocated.
	VFrame();
	~VFrame();

// Reallocate a frame without deleting the class
	int reallocate(unsigned char *data, 
		int w, 
		int h, 
		int color_model, 
		long bytes_per_line);

// if frame points to the same data as this return 1
	int equals(VFrame *frame);

// direct copy with no alpha
	int copy_from(VFrame *frame);
// Direct copy with alpha from 0 to VMAX.
	int replace_from(VFrame *frame, int alpha, int use_alpha, int use_float);
	static int get_scale_tables(int *column_table, int *row_table, 
			int in_x1, int in_y1, int in_x2, int in_y2,
			int out_x1, int out_y1, int out_x2, int out_y2);

	int apply_fade(int factor);
	int clear_frame();
	int allocate_compressed_data(long bytes);

	long get_compressed_allocated();
	long get_compressed_size();
	long set_compressed_size(long size);
	int get_color_model();
// Get the data pointer
	unsigned char* get_data();
// return an array of pointers to rows
	unsigned char** get_rows();       
// return yuv planes
	unsigned char* get_y(); 
	unsigned char* get_u(); 
	unsigned char* get_v(); 
	int get_w();
	int get_h();
	static int clear_pixel(VPixel &pixel);
	static int bytes_per_pixel(int color_model);
	long set_shm_offset(long offset);
	long get_shm_offset();

private:
	int clear_objects();
	int reset_parameters();
	int allocate_data(unsigned char *data, 
		int w, 
		int h, 
		int color_model, 
		long bytes_per_line);

// Data is pointing to someone else's buffer.
	int shared; 
// If not set by user, is calculated from VWORD
	long bytes_per_line;
// Image data
	unsigned char *data;
// Pointers to the start of each row
	unsigned char **rows;
// One of the #defines
	int color_model;
// Allocated space for compressed data
	long compressed_allocated;
// Size of stored compressed image
	long compressed_size;   
// Pointers to yuv planes
	unsigned char *y, *u, *v;
// Dimensions of frame
	int w, h;
// Extra information for shared memory
	long shm_offset;
};


#endif
