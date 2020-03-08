#include <png.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "clip.h"
#include "colormodels.h"
#include "vframe.h"

class PngReadFunction
{
public:
	static void png_read_function(png_structp png_ptr,
            	   png_bytep data, 
				   png_size_t length)
	{
		VFrame *frame = (VFrame*)png_get_io_ptr(png_ptr);
		if(frame->image_size - frame->image_offset < length) 
			length = frame->image_size - frame->image_offset;

		memcpy(data, &frame->image[frame->image_offset], length);
		frame->image_offset += length;
	};
};










VFrame::VFrame(VFrame &frame)
{
	reset_parameters();
	allocate_data(0, 0, 0, 0, frame.w, frame.h, frame.color_model, frame.bytes_per_line);
	memcpy(data, frame.data, bytes_per_line * h);
}

VFrame::VFrame(unsigned char *png_data)
{
	reset_parameters();
	read_png(png_data);
}

VFrame::VFrame(unsigned char *data, 
	int w, 
	int h, 
	int color_model, 
	long bytes_per_line)
{
	reset_parameters();
	allocate_data(data, 0, 0, 0, w, h, color_model, bytes_per_line);
}

VFrame::VFrame(unsigned char *data, 
		long y_offset,
		long u_offset,
		long v_offset, 
		int w, 
		int h, 
		int color_model, 
		long bytes_per_line)
{
	reset_parameters();
	allocate_data(data, 
		y_offset, 
		u_offset, 
		v_offset, 
		w, 
		h, 
		color_model, 
		bytes_per_line);
}

VFrame::VFrame()
{
	reset_parameters();
	this->color_model = BC_COMPRESSED;
}











VFrame::~VFrame()
{
	clear_objects();
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

int VFrame::params_match(int w, int h, int color_model)
{
	return (this->w == w &&
		this->h == h &&
		this->color_model == color_model);
}


int VFrame::reset_parameters()
{
	field2_offset = 0;
	shared = 0;
	shm_offset = 0;
	bytes_per_line = 0;
	data = 0;
	rows = 0;
	color_model = 0;
	compressed_allocated = 0;
	compressed_size = 0;   // Size of current image
	w = 0;
	h = 0;
	y = u = v = 0;
	y_offset = 0;
	u_offset = 0;
	v_offset = 0;
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
		case BC_COMPRESSED:
		case BC_YUV420P:
			break;

		default:
			delete [] rows;
			break;
	}

	return 0;
}

int VFrame::get_field2_offset()
{
	return field2_offset;
}

int VFrame::set_field2_offset(int value)
{
	this->field2_offset = value;
	return 0;
}

int VFrame::calculate_bytes_per_pixel(int color_model)
{
	return cmodel_calculate_pixelsize(color_model);
}

long VFrame::get_bytes_per_line()
{
	return bytes_per_line;
}

long VFrame::get_data_size()
{
	return h * bytes_per_line;
}

long VFrame::calculate_data_size(int w, int h, int bytes_per_line, int color_model)
{
	if(bytes_per_line < 0) bytes_per_line = w * calculate_bytes_per_pixel(color_model);
	switch(color_model)
	{
		case BC_YUV420P:
		case BC_YUV411P:
			return w * h + w * h / 2 + 4;
			break;

		case BC_YUV422P:
			return w * h * 2 + 4;
			break;

		default:
			return h * bytes_per_line + 4;
			break;
	}
	return 0;
}

int VFrame::allocate_data(unsigned char *data, 
	long y_offset,
	long u_offset,
	long v_offset,
	int w, 
	int h, 
	int color_model, 
	long bytes_per_line)
{
	this->w = w;
	this->h = h;
	this->color_model = color_model;
	this->bytes_per_pixel = calculate_bytes_per_pixel(color_model);
	this->y_offset = this->u_offset = this->v_offset = 0;

	if(bytes_per_line >= 0)
	{
		this->bytes_per_line = bytes_per_line;
	}
	else
		this->bytes_per_line = this->bytes_per_pixel * w;

// Allocate data + padding for MMX
	if(data)
	{
		shared = 1;
		this->data = data;
		this->y_offset = y_offset;
		this->u_offset = u_offset;
		this->v_offset = v_offset;
	}
	else
	{
		shared = 0;
		this->data = new unsigned char[calculate_data_size(this->w, this->h, this->bytes_per_line, this->color_model)];
	}

// Create row pointers
	switch(color_model)
	{
		case BC_YUV420P:
		case BC_YUV411P:
			if(!this->v_offset)
			{
				this->y_offset = 0;
				this->u_offset = w * h;
				this->v_offset = w * h + w * h / 4;
			}
			y = this->data + this->y_offset;
			u = this->data + this->u_offset;
			v = this->data + this->v_offset;
			break;

		case BC_YUV422P:
			if(!this->v_offset)
			{
				this->y_offset = 0;
				this->u_offset = w * h;
				this->v_offset = w * h + w * h / 2;
			}
			y = this->data + this->y_offset;
			u = this->data + this->u_offset;
			v = this->data + this->v_offset;
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
}

void VFrame::set_memory(unsigned char *data, 
		long y_offset,
		long u_offset,
		long v_offset)
{
	shared = 1;
	this->data = data;
	this->y_offset = y_offset;
	this->u_offset = u_offset;
	this->v_offset = v_offset;
	y = this->data + this->y_offset;
	u = this->data + this->u_offset;
	v = this->data + this->v_offset;
}

// Reallocate uncompressed buffer with or without alpha
int VFrame::reallocate(unsigned char *data, 
		long y_offset,
		long u_offset,
		long v_offset,
		int w, 
		int h, 
		int color_model, 
		long bytes_per_line)
{
	clear_objects();
	reset_parameters();
	allocate_data(data, y_offset, u_offset, v_offset, w, h, color_model, bytes_per_line);
	return 0;
}

int VFrame::allocate_compressed_data(long bytes)
{
	if(bytes < 1) return 1;

// Want to preserve original contents
	if(data && compressed_allocated < bytes)
	{
//printf("VFrame::allocate_compressed_data 1 %d\n", bytes);
		unsigned char *new_data = new unsigned char[bytes];
//printf("VFrame::allocate_compressed_data 1\n");
		bcopy(data, new_data, compressed_allocated);
//printf("VFrame::allocate_compressed_data 1\n");
		delete [] data;
//printf("VFrame::allocate_compressed_data 1\n");
		data = new_data;
		compressed_allocated = bytes;
//printf("VFrame::allocate_compressed_data 2\n");
	}
	else
	if(!data)
	{
		data = new unsigned char[bytes];
		compressed_allocated = bytes;
		compressed_size = 0;
	}

	return 0;
}

int VFrame::read_png(unsigned char *data)
{
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	int new_color_model;

	image_offset = 0;
	image = data + 4;
	image_size = (((unsigned long)data[0]) << 24) | 
		(((unsigned long)data[1]) << 16) | 
		(((unsigned long)data[2]) << 8) | 
		(unsigned char)data[3];
	png_set_read_fn(png_ptr, this, PngReadFunction::png_read_function);
	png_read_info(png_ptr, info_ptr);

	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);

	switch(png_get_color_type(png_ptr, info_ptr))
	{
		case PNG_COLOR_TYPE_RGB:
			new_color_model = BC_RGB888;
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
		default:
			new_color_model = BC_RGBA8888;
			break;
	}

	reallocate(NULL, 
		0, 
		0, 
		0, 
		w, 
		h, 
		new_color_model,
		-1);

	png_read_image(png_ptr, get_rows());
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
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
}


int VFrame::equals(VFrame *frame)
{
	if(frame->data == data) 
		return 1;
	else
		return 0;
}

int VFrame::clear_frame()
{
	switch(color_model)
	{
		case BC_COMPRESSED:
			break;

		case BC_YUV420P:
			bzero(data, h * w * 2);
			break;

		default:
			bzero(data, h * bytes_per_line);
			break;
	}
	return 0;
}

void VFrame::rotate90()
{
// Allocate new frame
	int new_w = h, new_h = w, new_bytes_per_line = bytes_per_pixel * new_w;
	unsigned char *new_data = new unsigned char[calculate_data_size(new_w, new_h, new_bytes_per_line, color_model)];
	unsigned char **new_rows = new unsigned char*[new_h];
	for(int i = 0; i < new_h; i++)
		new_rows[i] = &new_data[new_bytes_per_line * i];

// Copy data
	for(int in_y = 0, out_x = new_w - 1; in_y < h; in_y++, out_x--)
	{
		for(int in_x = 0, out_y = 0; in_x < w; in_x++, out_y++)
		{
			for(int k = 0; k < bytes_per_pixel; k++)
			{
				new_rows[out_y][out_x * bytes_per_pixel + k] = 
					rows[in_y][in_x * bytes_per_pixel + k];
			}
		}
	}

// Swap frames
	clear_objects();
	data = new_data;
	rows = new_rows;
	bytes_per_line = new_bytes_per_line;
	w = new_w;
	h = new_h;
}

void VFrame::rotate270()
{
// Allocate new frame
	int new_w = h, new_h = w, new_bytes_per_line = bytes_per_pixel * new_w;
	unsigned char *new_data = new unsigned char[calculate_data_size(new_w, new_h, new_bytes_per_line, color_model)];
	unsigned char **new_rows = new unsigned char*[new_h];
	for(int i = 0; i < new_h; i++)
		new_rows[i] = &new_data[new_bytes_per_line * i];

// Copy data
	for(int in_y = 0, out_x = 0; in_y < h; in_y++, out_x++)
	{
		for(int in_x = 0, out_y = new_h - 1; in_x < w; in_x++, out_y--)
		{
			for(int k = 0; k < bytes_per_pixel; k++)
			{
				new_rows[out_y][out_x * bytes_per_pixel + k] = 
					rows[in_y][in_x * bytes_per_pixel + k];
			}
		}
	}

// Swap frames
	clear_objects();
	data = new_data;
	rows = new_rows;
	bytes_per_line = new_bytes_per_line;
	w = new_w;
	h = new_h;
}

void VFrame::flip_vert()
{
	for(int i = 0, j = h - 1; i < j; i++, j--)
	{
		for(int k = 0; k < bytes_per_line; k++)
		{
			unsigned char temp = rows[j][k];
			rows[j][k] = rows[i][k];
			rows[i][k] = temp;
		}
	}
}

int VFrame::apply_fade(float alpha)
{

	if(alpha != 1)
	{
		switch(color_model)
		{
			case BC_YUVA8888:
			case BC_RGBA8888:
				for(int i = 0; i < h; i++)
				{
					for(int j = 0; j < w; j++)
					{
						float channel = rows[i][j * 4 + 3] * alpha;
						rows[i][j * 4 + 3] = (unsigned char)channel;
					}
				}
				break;

			case BC_YUVA16161616:
			case BC_RGBA16161616:
				for(int i = 0; i < h; i++)
				{
					for(int j = 0; j < w; j++)
					{
						float channel = ((u_int16_t*)rows[i])[j * 4 + 3] * alpha;
						rows[i][j * 4 + 3] = (u_int16_t)channel;
					}
				}
				break;
		}
	}
	return 0;
}

int VFrame::replace_from(VFrame *frame, float alpha)
{
	int use_direct_copy = 1;
	if(alpha != 1)
	{
		int a_int = (int)CLIP(alpha * 255 + 0.5, 0, 255);

		switch(color_model)
		{
			case BC_RGBA8888:
			case BC_YUVA8888:
			{
				unsigned char **dst_rows = rows;
				unsigned char **src_rows = frame->get_rows();
				for(int i = 0; i < h; i++)
				{
					for(int j = 0; j < w; j++)
					{
						dst_rows[i][j * 4 + 0] = src_rows[i][j * 4 + 0];
						dst_rows[i][j * 4 + 1] = src_rows[i][j * 4 + 1];
						dst_rows[i][j * 4 + 2] = src_rows[i][j * 4 + 2];
						dst_rows[i][j * 4 + 3] = (unsigned char)(src_rows[i][j * 4 + 3] * alpha + 0.5);
					}
				}
				use_direct_copy = 0;
			}
				break;

			case BC_RGBA16161616:
			case BC_YUVA16161616:
			{
				unsigned char **dst_rows = rows;
				unsigned char **src_rows = frame->get_rows();
				for(int i = 0; i < h; i++)
				{
					for(int j = 0; j < w; j++)
					{
						((u_int16_t*)dst_rows[i])[j * 4 + 0] = ((u_int16_t*)src_rows[i])[j * 4 + 0];
						((u_int16_t*)dst_rows[i])[j * 4 + 1] = ((u_int16_t*)src_rows[i])[j * 4 + 1];
						((u_int16_t*)dst_rows[i])[j * 4 + 2] = ((u_int16_t*)src_rows[i])[j * 4 + 2];
						((u_int16_t*)dst_rows[i])[j * 4 + 3] = (unsigned short)(((u_int16_t*)src_rows[i])[j * 4 + 3] * alpha + 0.5);
					}
				}
				use_direct_copy = 0;
			}
				break;
		}
	}

	if(use_direct_copy) copy_from(frame);

	return 0;
}

int VFrame::copy_from(VFrame *frame)
{
	switch(frame->color_model)
	{
		case BC_COMPRESSED:
			allocate_compressed_data(frame->compressed_size);
			memcpy(data, frame->data, frame->compressed_size);
			this->compressed_size = frame->compressed_size;
			break;

		case BC_YUV420P:
//printf("%d %d %p %p %p %p %p %p\n", w, h, get_y(), get_u(), get_v(), frame->get_y(), frame->get_u(), frame->get_v());
			memcpy(get_y(), frame->get_y(), w * h);
			memcpy(get_u(), frame->get_u(), w * h / 4);
			memcpy(get_v(), frame->get_v(), w * h / 4);
			break;

		case BC_YUV422P:
//printf("%d %d %p %p %p %p %p %p\n", w, h, get_y(), get_u(), get_v(), frame->get_y(), frame->get_u(), frame->get_v());
			memcpy(get_y(), frame->get_y(), w * h);
			memcpy(get_u(), frame->get_u(), w * h / 2);
			memcpy(get_v(), frame->get_v(), w * h / 2);
			break;

		default:
// printf("VFrame::copy_from %d\n", calculate_data_size(w, 
// 				h, 
// 				-1, 
// 				frame->color_model));
			memcpy(data, frame->data, calculate_data_size(w, 
				h, 
				-1, 
				frame->color_model));
			break;
	}

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
}

int VFrame::get_bytes_per_pixel()
{
	return bytes_per_pixel;
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
}

int VFrame::get_h()
{
	return h;
}

int VFrame::get_w_fixed()
{
	return w - 1;
}

int VFrame::get_h_fixed()
{
	return h - 1;
}

unsigned char* VFrame::get_y()
{
	return y;
}

unsigned char* VFrame::get_u()
{
	return u;
}

unsigned char* VFrame::get_v()
{
	return v;
}

