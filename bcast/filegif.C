#include <string.h>
#include "assets.h"
#include "file.h"
#include "filetiff.h"
#include "vframe.h"


FileGIF::FileGIF(Asset *asset) : FileBase(asset)
{
	reset_parameters();
	asset->video_data = 1;
	asset->format = FILE_GIF;
}

FileGIF::~FileGIF()
{
	close_file();
}

int FileGIF::reset_parameters_derived()
{
	data = 0;
return 0;
}

int FileGIF::open_file(int rd, int wr)
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

long FileGIF::get_video_length()
{
	return -1;    // infinity
// should be determined by whether the GIF is animated
}

long FileGIF::get_memory_usage()
{
// give buffer length plus padding
	if(data)
		return asset->width * asset->height * sizeof(VPixel);
	else
		return 256;
}

int FileGIF::close_file_derived()
{
	if(data) delete data;
	reset_parameters();
return 0;
}

int FileGIF::read_header()
{
	GIF *stream;

	if(!(stream = GIFOpen(asset->path, "r")))
	{
		perror("FileGIF::read_header");
		return 1;
	}
	
	GIFGetField(stream, GIFTAG_IMAGEWIDTH, &(asset->width));
	GIFGetField(stream, GIFTAG_IMAGELENGTH, &(asset->height));
	asset->layers = 1;

	GIFClose(stream);
	return 0;
return 0;
}

int FileGIF::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int use_alpha, int use_float)
{
	read_raw();
	frame->transfer_from(this->data, in_x1, in_y1, in_x2, in_y2,
						out_x1, out_y1, out_x2, out_y2, alpha,
						use_alpha, use_float);
return 0;
}

VFrame* FileGIF::read_frame(int use_alpha, int use_float)
{
	read_raw();
	return data;
}

int FileGIF::read_raw()
{
	if(!data)
	{
// read the raw data
		GIF *stream;
		unsigned char *raw_data;
		int i;

		if(!(stream = GIFOpen(asset->path, "r")))
		{
			perror("FileGIF::read_raw");
			return 1;
		}

		raw_data = new unsigned char[asset->width * asset->height * 4];
		GIFReadRGBAImage(stream, asset->width, asset->height, (uint32*)raw_data, 0);

		GIFClose(stream);

// convert to a Bcast 2000 Frame
		data = new VFrame(asset->width, asset->height);

		for(i = 0; i < asset->height; i++)
		{
			import_row(data->rows[asset->height - i - 1], &raw_data[i * asset->width * 4]);
		}

// delete temporary buffers
		delete raw_data;
	}
	return 0;
return 0;
}

int FileGIF::import_row(VPixel *output, unsigned char *row_pointer)
{
	for(int i = 0, j = 0; j < asset->width; j++)
	{
#if (VMAX == 65535)
		output[j].red =  ((VWORD)row_pointer[i++]) << 8;
		output[j].green =  ((VWORD)row_pointer[i++]) << 8;
		output[j].blue = ((VWORD)row_pointer[i++]) << 8;
		output[j].alpha = ((VWORD)row_pointer[i++]) << 8;
#else
		output[j].red =  (VWORD)row_pointer[i++];
		output[j].green =  (VWORD)row_pointer[i++];
		output[j].blue = (VWORD)row_pointer[i++];
		output[j].alpha = (VWORD)row_pointer[i++];
#endif
	}
return 0;
}
