#include <string.h>
#include "assets.h"
#include "file.h"
#include "filepng.h"
#include "vframe.h"


FilePNG::FilePNG(Asset *asset, File *file) : FileBase(asset, file)
{
	reset_parameters();
	asset->video_data = 1;
	asset->format = PNG;
}

FilePNG::~FilePNG()
{
	close_file();
}

int FilePNG::reset_parameters_derived()
{
	stream = 0;
	png_ptr = 0;
	info_ptr = 0;
	end_info = 0;
	data = 0;
	data_len = 0;
return 0;
}

int FilePNG::open_file(int rd, int wr)
{
	this->rd = rd;
	this->wr = wr;
	char mode[32];

	get_mode(mode, rd, wr);
	if(!(stream = fopen(asset->path, mode)))
	{
		perror("FilePNG::open_file");
		return 1;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, stream);

// skip header for write
	if(wr)
	{
	}
	else
	if(rd)
	{
		png_read_info(png_ptr, info_ptr);
		read_header();
	}
	return 0;
return 0;
}

long FilePNG::get_video_length()
{
	return -1;    // infinity
}

long FilePNG::get_memory_usage()
{
// give buffer length plus padding
	return data_len * sizeof(VPixel) + 256;
}

int FilePNG::close_file_derived()
{
	if(stream)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		fclose(stream);
	}

	if(data) delete data;
return 0;
}

int FilePNG::read_header()
{
	asset->width = png_get_image_width(png_ptr, info_ptr);
	asset->height = png_get_image_height(png_ptr, info_ptr);
	asset->layers = 1;
	asset->frame_rate = 1;
return 0;
}

int FilePNG::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	read_raw();
	transfer_from(frame, this->data, in_x1, in_y1, in_x2, in_y2,
						out_x1, out_y1, out_x2, out_y2, alpha,
						use_alpha, use_float, interpolate, NORMAL);
return 0;
}

VFrame* FilePNG::read_frame(int use_alpha, int use_float)
{
	read_raw();
	return data;
}




#define IMPORT1 \
	for(int i = 0; i < asset->height; i++) \
	{

#define IMPORT2 \
	}

int FilePNG::read_raw()
{
	if(!data)
	{
// read the raw data
		png_bytep *row_pointers;
		int row_bytes;
		int bit_depth;
		int color_type;

		row_pointers = new png_bytep[asset->height];
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		row_bytes = png_get_rowbytes(png_ptr, info_ptr);

		init_row_pointers(row_pointers, row_bytes);

		png_read_image(png_ptr, row_pointers);

// convert to a Bcast 2000 Frame
		data = new VFrame(0, asset->width, asset->height);
		data_len = asset->width * asset->height;

		switch(color_type)
		{
			case PNG_COLOR_TYPE_GRAY:
				IMPORT1
					import_row_grey(((VPixel**)data->get_rows())[i], row_pointers[i], bit_depth);
				IMPORT2
				break;

			case PNG_COLOR_TYPE_GRAY_ALPHA:
				IMPORT1
					import_row_grey_alpha(((VPixel**)data->get_rows())[i], row_pointers[i], bit_depth);
				IMPORT2
				break;

			case PNG_COLOR_TYPE_PALETTE:
				{
					int num_palette;
					png_color *palette;

					png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

					IMPORT1
						import_row_palette(((VPixel**)data->get_rows())[i], row_pointers[i], bit_depth, palette, num_palette);
					IMPORT2
				}
				break;

			case PNG_COLOR_TYPE_RGB:
				IMPORT1
					import_row_rgb(((VPixel**)data->get_rows())[i], row_pointers[i], bit_depth);
				IMPORT2
				break;

			case PNG_COLOR_TYPE_RGB_ALPHA:
				IMPORT1
					import_row_rgb_alpha(((VPixel**)data->get_rows())[i], row_pointers[i], bit_depth);
				IMPORT2
				break;
		};

		delete_row_pointers(row_pointers);
		delete [] row_pointers;
	}
return 0;
}

int FilePNG::init_row_pointers(png_bytep *row_pointers, int row_bytes)
{
	for(int i = 0; i < asset->height; i++)
	{
		row_pointers[i] = new png_byte[row_bytes];
	}
return 0;
}

int FilePNG::delete_row_pointers(png_bytep *row_pointers)
{
	for(int i = 0; i < asset->height; i++)
	{
		delete row_pointers[i];
	}
return 0;
}

int FilePNG::import_row_grey(VPixel *output, png_bytep row_pointer, int bit_depth)
{
	switch(bit_depth)
	{
		case 1:
		{
			int i = 0, j = 0, bit = 128;
			while(j < asset->width)
			{
				output[j].r = output[j].g = output[j].b = (row_pointer[i] & bit) ? VMAX : 0;
				output[j].a = VMAX;
				if(bit == 1) { bit = 128; i++; } else bit /= 2;
				j++;
			}
		}
			break;

		case 2:
		{
			int roll[4];
			roll[0] = 6;
			roll[1] = 4;
			roll[2] = 2;
			roll[3] = 0;
			int i = 0, j = 0, k = 0, bit = 192;
			
			while(j < asset->width)
			{
				output[j].r = row_pointer[i] & bit;
				output[j].r >>= roll[k];
				output[j].r *= VMAX / 3;
				output[j].g = output[j].b = output[j].r;
				output[j].a = VMAX;
				j++;
				if(k == 3) { k = 0; bit = 192; i++; } else { k++; bit >>= 2; }
			}
		}
			break;

		case 4:
		{
			int roll[2];
			roll[0] = 4;
			roll[1] = 0;
			int i = 0, j = 0, k = 0, bit = 240;
			
			while(j < asset->width)
			{
				output[j].r = row_pointer[i] & bit;
				output[j].r >>= roll[k];
				output[j].r *= VMAX / 15;
				output[j].g = output[j].b = output[j].r;
				output[j].a = VMAX;
				j++;
				if(k == 1) { k = 0; bit = 240; i++; } else { k++; bit >>= 4; }
			}
		}
			break;

		case 8:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = output[j].g = output[j].b = row_pointer[i] * VMAX / 255;
				output[j].a = VMAX;
			}
		}
			break;

		case 16:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = output[j].g = output[j].b = row_pointer[i] / (65535 / VMAX);
				output[j].a = VMAX;
			}
		}
			break;
	}
return 0;
}

int FilePNG::import_row_grey_alpha(VPixel *output, png_bytep row_pointer, int bit_depth)
{
	switch(bit_depth)
	{
		case 8:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = output[j].g = output[j].b = row_pointer[i++] * VMAX / 255;
				output[j].a = row_pointer[i] * (VMAX / 255);
			}
		}
			break;

		case 16:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = output[j].g = output[j].b = row_pointer[i++] / (65535 / VMAX);
				output[j].a = row_pointer[i] / (65535 / VMAX);
			}
		}
			break;
	}
return 0;
}

int FilePNG::import_row_rgb(VPixel *output, png_bytep row_pointer, int bit_depth)
{
	switch(bit_depth)
	{
		case 8:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = row_pointer[i++] * VMAX / 255;
				output[j].g =  row_pointer[i++] * VMAX / 255;
				output[j].b =  row_pointer[i] * VMAX / 255;
				output[j].a = VMAX;
			}
		}
			break;

		case 16:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = row_pointer[i++] / (65535 / VMAX);
				output[j].g = row_pointer[i++] / (65535 / VMAX);
				output[j].b = row_pointer[i] / (65535 / VMAX);
				output[j].a = VMAX;
			}
		}
			break;
	}
return 0;
}


int FilePNG::import_row_rgb_alpha(VPixel *output, png_bytep row_pointer, int bit_depth)
{
	switch(bit_depth)
	{
		case 8:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = row_pointer[i++] * VMAX / 255;
				output[j].g =  row_pointer[i++] * VMAX / 255;
				output[j].b =  row_pointer[i++] * VMAX / 255;
				output[j].a = row_pointer[i] * VMAX / 255;
			}
		}
			break;

		case 16:
		{
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				output[j].r = row_pointer[i++] / (65535 / VMAX);
				output[j].g = row_pointer[i++] / (65535 / VMAX);
				output[j].b = row_pointer[i++] / (65535 / VMAX);
				output[j].a = row_pointer[i] / (65535 / VMAX);
			}
		}
			break;
	}
return 0;
}

int FilePNG::import_row_palette(VPixel *output, png_bytep row_pointer, int bit_depth, png_color *palette, int num_palette)
{
	switch(bit_depth)
	{
		case 1:
		{
			int i = 0, j = 0, bit = 128, value;
			while(j < asset->width)
			{
				value = (row_pointer[i] & bit) ? 1 : 0;
				output[j].r = palette[value].red;
				output[j].g = palette[value].green;
				output[j].b = palette[value].blue;
				output[j].a = VMAX;
				if(bit == 1) { bit = 128; i++; } else bit /= 2;
				j++;
			}
		}
			break;

		case 2:
		{
			int roll[4];
			roll[0] = 6;
			roll[1] = 4;
			roll[2] = 2;
			roll[3] = 0;
			int i = 0, j = 0, k = 0, bit = 192, value;
			
			while(j < asset->width)
			{
				value = row_pointer[i] & bit;
				value >>= roll[k];
				output[j].r = palette[value].red;
				output[j].g = palette[value].green;
				output[j].b = palette[value].blue;
				output[j].a = VMAX;
				j++;
				if(k == 3) { k = 0; bit = 192; i++; } else { k++; bit >>= 2; }
			}
		}
			break;

		case 4:
		{
			int roll[2];
			roll[0] = 4;
			roll[1] = 0;
			int i = 0, j = 0, k = 0, bit = 240, value;
			
			while(j < asset->width)
			{
				value = row_pointer[i] & bit;
				value >>= roll[k];
				output[j].r = palette[value].red;
				output[j].g = palette[value].green;
				output[j].b = palette[value].blue;
				output[j].a = VMAX;
				j++;
				if(k == 1) { k = 0; bit = 240; i++; } else { k++; bit >>= 4; }
			}
		}
			break;

		case 8:
		{
			int value;
			for(int i = 0, j = 0; j < asset->width; i++, j++)
			{
				value = row_pointer[i];
				output[j].r = palette[value].red;
				output[j].g = palette[value].green;
				output[j].b =palette[value].blue;
				output[j].a = VMAX;
			}
		}
			break;
	}
return 0;
}
