#include <string.h>
#include "assets.h"
#include "file.h"
#include "filetiff.h"
#include "vframe.h"


FileTIFF::FileTIFF(Asset *asset, File *file) : FileBase(asset, file)
{
	reset_parameters();
	asset->video_data = 1;
	asset->format = FILE_TIFF;
}

FileTIFF::~FileTIFF()
{
	close_file();
}

int FileTIFF::reset_parameters_derived()
{
	data = 0;
return 0;
}

int FileTIFF::open_file(int rd, int wr)
{
	this->rd = rd;
	this->wr = wr;

// skip header for write
	if(wr)
	{
	}
	else
	if(rd)
	{
		return read_header();
	}
	return 0;
return 0;
}

long FileTIFF::get_video_length()
{
	return -1;    // infinity
}

long FileTIFF::get_memory_usage()
{
// give buffer length plus padding
	if(data)
		return asset->width * asset->height * sizeof(VPixel);
	else
		return 256;
}

int FileTIFF::close_file_derived()
{
	if(data) delete data;
	reset_parameters();
return 0;
}

int FileTIFF::read_header()
{
	TIFF *stream;

	if(!(stream = TIFFOpen(asset->path, "r")))
	{
		perror("FileTIFF::read_header");
		return 1;
	}

	TIFFGetField(stream, TIFFTAG_IMAGEWIDTH, &(asset->width));
	TIFFGetField(stream, TIFFTAG_IMAGELENGTH, &(asset->height));
	asset->layers = 1;
	asset->frame_rate = 1;

	TIFFClose(stream);
	return 0;
return 0;
}

int FileTIFF::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
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

VFrame* FileTIFF::read_frame(int use_alpha, int use_float)
{
	read_raw();
	return data;
}

int FileTIFF::read_raw()
{
	if(!data)
	{
// read the raw data
		TIFF *stream;
		unsigned char *raw_data;
		int i;

		if(!(stream = TIFFOpen(asset->path, "r")))
		{
			perror("FileTIFF::read_raw");
			return 1;
		}

		raw_data = new unsigned char[asset->width * asset->height * 4];
		TIFFReadRGBAImage(stream, asset->width, asset->height, (uint32*)raw_data, 0);

		TIFFClose(stream);

// convert to a V Frame
		data = new VFrame(0, asset->width, asset->height);

		for(i = 0; i < asset->height; i++)
		{
			import_row(((VPixel**)data->get_rows())[asset->height - i - 1], &raw_data[i * asset->width * 4]);
		}

// delete temporary buffers
		delete raw_data;
	}
	return 0;
return 0;
}

int FileTIFF::import_row(VPixel *output, unsigned char *row_pointer)
{
	for(int i = 0, j = 0; j < asset->width; j++)
	{
#if (VMAX == 65535)
		output[j].r =  ((VWORD)row_pointer[i++]) << 8;
		output[j].g =  ((VWORD)row_pointer[i++]) << 8;
		output[j].b = ((VWORD)row_pointer[i++]) << 8;
		output[j].a = ((VWORD)row_pointer[i++]) << 8;
#else
		output[j].r =  (VWORD)row_pointer[i++];
		output[j].g =  (VWORD)row_pointer[i++];
		output[j].b = (VWORD)row_pointer[i++];
		output[j].a = (VWORD)row_pointer[i++];
#endif
	}
return 0;
}
