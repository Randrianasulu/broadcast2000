#include "vframe.h"
#include <stdio.h>
#include <string.h>

VFrame::VFrame(unsigned char *data, int w, int h, int color_model, long bytes_per_line)
{
	int i;
	reset_parameters();
	allocate_data(data, w, h, color_model, bytes_per_line);
}

VFrame::VFrame()
{
	reset_parameters();
	this->color_model = VFRAME_COMPRESSED;
}

VFrame::~VFrame()
{
	clear_objects();
}

int VFrame::reset_parameters()
{
	shared = 0;
	bytes_per_line = 0;
	data = 0;
	rows = 0;
	color_model = 0;
	compressed_allocated = 0;
	compressed_size = 0;   // Size of current image
	w = 0;
	h = 0;
	return 0;
return 0;
}

int VFrame::clear_objects()
{
// Delete data
	if(!shared)
	{
		if(data) delete [] data;
		data = 0;
	}

// Delete row pointers
	switch(color_model)
	{
		case VFRAME_COMPRESSED:
		case VFRAME_YUV420:
			break;

		default:
			delete [] rows;
			break;
	}

	return 0;
return 0;
}

int VFrame::bytes_per_pixel(int color_model)
{
	switch(color_model)
	{
		case VFRAME_YUV420:
			return 0;
			break;

		case VFRAME_RGB565:
			return 2;
			break;

		case VFRAME_BGRA8880:
		case VFRAME_RGBA8880:
			return 4;
			break;

		case VFRAME_BGR888:
		case VFRAME_RGB888:
			return 3;
			break;

		case VFRAME_VPIXEL:
			return sizeof(VPixel);
			break;

		case VFRAME_VPIXELYUV:
			return sizeof(VPixelYUV);
			break;

		default:
			return 0;
			break;
	}
	return 0;
return 0;
}

int VFrame::allocate_data(unsigned char *data, 
	int w, 
	int h, 
	int color_model, 
	long bytes_per_line)
{

	this->w = w;
	this->h = h;
	this->color_model = color_model;

// Get default bytes per line
	if(bytes_per_line < 0)
	{
		this->bytes_per_line = bytes_per_pixel(color_model) * w;
	}
	else
		this->bytes_per_line = bytes_per_line;

// Allocate data + padding for MMX
	if(data)
	{
		shared = 1;
		this->data = data;
	}
	else
	{
		shared = 0;
		switch(color_model)
		{
			case VFRAME_YUV420:
				this->data = new unsigned char[w * h * 2 + 4];
				break;
				
			default:
				this->data = new unsigned char[h * this->bytes_per_line + 4];
				break;
		}
	}

// Create row pointers
	switch(color_model)
	{
		case VFRAME_YUV420:
			y = this->data;
			u = this->data + w * h;
			v = this->data + w * h + w * h / 2;
			break;

		default:
			rows = new unsigned char*[h];
			for(int i = 0; i < h; i++)
			{
				rows[i] = &this->data[i * this->bytes_per_line];
			}
			break;
	}
	return 0;
return 0;
}


// Reallocate uncompressed buffer with or without alpha
int VFrame::reallocate(unsigned char *data, 
		int w, 
		int h, 
		int color_model, 
		long bytes_per_line)
{
	clear_objects();
	reset_parameters();
	allocate_data(data, w, h, color_model, bytes_per_line);
	return 0;
return 0;
}

int VFrame::allocate_compressed_data(long bytes)
{
	if(bytes < 1) return 1;

	if(data && compressed_allocated < bytes)
	{
		delete [] data;
		data = 0;
	}

	if(!data)
	{
		data = new unsigned char[bytes];
		compressed_allocated = bytes;
		compressed_size = 0;
	}

	return 0;
return 0;
}

unsigned char* VFrame::get_data()
{
	return data;
}

long VFrame::get_compressed_allocated()
{
	return compressed_allocated;
}

long VFrame::get_compressed_size()
{
	return compressed_size;
}

long VFrame::set_compressed_size(long size)
{
	compressed_size = size;
	return 0;
}

int VFrame::get_color_model()
{
	return color_model;
return 0;
}


int VFrame::equals(VFrame *frame)
{
	if(frame->data == data) 
		return 1;
	else
		return 0;
return 0;
}

int VFrame::clear_frame()
{
	switch(color_model)
	{
		case VFRAME_COMPRESSED:
			break;
			
		case VFRAME_YUV420:
			memset(data, 0, h * w * 2);
			break;

		default:
			memset(data, 0, h * bytes_per_line);
			break;
	}
	return 0;
return 0;
}

int VFrame::apply_fade(int alpha)
{
	 int i, j, channel;

	if(alpha != VMAX)
	{
		switch(color_model)
		{
			case VFRAME_VPIXEL:
			{
				VPixel **rows_rgb = (VPixel**)rows;
				for(i = 0; i < h; i++)
				{
					for(j = 0; j < w; j++)
					{
						channel = rows_rgb[i][j].a;
						channel *= alpha;
						channel >>= sizeof(VWORD) * 8;
						rows_rgb[i][j].a = (VWORD)channel;
					}
				}
			}
				break;

			case VFRAME_VPIXELYUV:
			{
				VPixelYUV **rows_yuv = (VPixelYUV**)rows;
				for(i = 0; i < h; i++)
				{
					for(j = 0; j < w; j++)
					{
						channel = rows_yuv[i][j].a;
						channel *= alpha;
						channel >>= sizeof(VWORD) * 8;
						rows_yuv[i][j].a = (VWORD)channel;
					}
				}
			}
				break;
		}
	}
	return 0;
return 0;
}

int VFrame::replace_from(VFrame *frame, int alpha, int use_alpha, int use_float)
{
	 long i, j;

	if(color_model == VFRAME_VPIXEL && alpha != VMAX)
	{
		float a_float;
		int a_int, channel;
		VPixel **dst_rows = (VPixel**)rows;
		VPixel **src_rows = (VPixel**)frame->get_rows();

		if(use_float)
		{
			a_float = (float)alpha / VMAX;
		}
		else
		{
			a_int = alpha;
#if (VMAX == 65535)
			a_int >>= 8;
#endif
		}

		for(i = 0; i < h; i++)
		{
			if(use_float)
			{
				for(j = 0; j < w; j++)
				{
					dst_rows[i][j].r = src_rows[i][j].r;
					dst_rows[i][j].g = src_rows[i][j].g;
					dst_rows[i][j].b = src_rows[i][j].b;
					dst_rows[i][j].a = (VWORD)(src_rows[i][j].a * a_float);
				}
			}
			else
			{
				for(j = 0; j < w; j++)
				{
					dst_rows[i][j].r = src_rows[i][j].r;
					dst_rows[i][j].g = src_rows[i][j].g;
					dst_rows[i][j].b = src_rows[i][j].b;
					dst_rows[i][j].a = (VWORD)(((int)src_rows[i][j].a * a_int) >> 8);
				}
			}
		}
	}
	else		
		memcpy(this->data, frame->data, h * bytes_per_line);

	return 0;
return 0;
}

int VFrame::copy_from(VFrame *frame)
{
	switch(frame->color_model)
	{
		case VFRAME_COMPRESSED:
			allocate_compressed_data(frame->compressed_size);
			memcpy(data, frame->data, frame->compressed_size);
			this->compressed_size = frame->compressed_size;
			break;

		case VFRAME_YUV420:
			memcpy(data, frame->data, w * h * 2);
			break;

		default:
			memcpy(data, frame->data, h * bytes_per_line);
			break;
	}

	return 0;
return 0;
}


int VFrame::get_scale_tables(int *column_table, int *row_table, 
			int in_x1, int in_y1, int in_x2, int in_y2,
			int out_x1, int out_y1, int out_x2, int out_y2)
{
	int y_out, i;
	float w_in = in_x2 - in_x1;
	float h_in = in_y2 - in_y1;
	int w_out = out_x2 - out_x1;
	int h_out = out_y2 - out_y1;

	float hscale = w_in / w_out;
	float vscale = h_in / h_out;

	for(i = 0; i < w_out; i++)
	{
		column_table[i] = (int)(hscale * i);
	}

	for(i = 0; i < h_out; i++)
	{
		row_table[i] = (int)(vscale * i) + in_y1;
	}
	return 0;
return 0;
}

int VFrame::clear_pixel(VPixel &pixel)
{
	pixel.r = pixel.g = pixel.b = pixel.a = 0;
return 0;
}

unsigned char** VFrame::get_rows()
{
	if(rows)
	{
		return rows;
	}
	return 0;
}

int VFrame::get_w()
{
	return w;
return 0;
}

int VFrame::get_h()
{
	return h;
return 0;
}

long VFrame::set_shm_offset(long offset)
{
	shm_offset = offset;
	return 0;
}

long VFrame::get_shm_offset()
{
	return shm_offset;
}

