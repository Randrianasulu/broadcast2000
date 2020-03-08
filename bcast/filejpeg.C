#include <string.h>
#include "assets.h"
#include "file.h"
#include "filejpeg.h"
#include "jpegwrapper.h"
#include "vframe.h"



FileJPEG::FileJPEG(Asset *asset, File *file) : FileBase(asset, file)
{
	reset_parameters();
	asset->video_data = 1;
	asset->format = JPEG;
}

FileJPEG::~FileJPEG()
{
	close_file();
}

int FileJPEG::reset_parameters_derived()
{
	data = 0;
return 0;
}

int FileJPEG::open_file(int rd, int wr)
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

long FileJPEG::get_video_length()
{
	return -1;    // infinity
}

long FileJPEG::get_memory_usage()
{
// give buffer length plus padding
	if(data)
		return asset->width * asset->height * sizeof(VPixel);
	else
		return 256;
}

int FileJPEG::close_file_derived()
{
	if(data) delete data;
	reset_parameters();
return 0;
}

int FileJPEG::read_header()
{
	FILE *stream;

	if(!(stream = fopen(asset->path, "rb")))
	{
		perror("FileJPEG::jpeg_read_header");
		return 1;
	}

	struct jpeg_decompress_struct jpeg_decompress;
	struct jpeg_error_mgr jpeg_error;

	jpeg_decompress.err = jpeg_std_error(&jpeg_error);
	jpeg_create_decompress(&jpeg_decompress);

	jpeg_stdio_src(&jpeg_decompress, stream);
	jpeg_read_header(&jpeg_decompress, TRUE);

	asset->width = jpeg_decompress.image_width;
	asset->height = jpeg_decompress.image_height;
	asset->layers = 1;
	asset->frame_rate = 1;

	jpeg_destroy((j_common_ptr)&jpeg_decompress);
	fclose(stream);
	return 0;
return 0;
}

int FileJPEG::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	read_raw(use_alpha, use_float);
	transfer_from(frame, this->data, in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, alpha, use_alpha, 
		use_float, interpolate, NORMAL);
return 0;
}

VFrame* FileJPEG::read_frame(int use_alpha, int use_float)
{
	read_raw(use_alpha, use_float);
	return data;
}

#define IMPORT1 \
	for(int i = 0; i < asset->height; i++) \
	{

#define IMPORT2 \
	}

int FileJPEG::read_raw(int use_alpha, int use_float)
{
	if(!data)
	{
// read the raw data
		FILE *stream;
		JSAMPLE *raw_data;
		JSAMPROW *row_pointers;
		struct jpeg_decompress_struct jpeg_decompress;
		struct jpeg_error_mgr jpeg_error;
		int i;
		int color_channels;

		if(!(stream = fopen(asset->path, "rb")))
		{
			perror("FileJPEG::jpeg_read_header");
			return 1;
		}

		jpeg_decompress.err = jpeg_std_error(&jpeg_error);
		jpeg_create_decompress(&jpeg_decompress);

		jpeg_stdio_src(&jpeg_decompress, stream);
		jpeg_read_header(&jpeg_decompress, TRUE);

		jpeg_start_decompress(&jpeg_decompress);
		color_channels = jpeg_decompress.jpeg_color_space;

		if(use_float) jpeg_decompress.dct_method = JDCT_FLOAT;

// allocate the temporary buffer
		raw_data = new JSAMPLE[asset->width * asset->height * color_channels];
		row_pointers = new JSAMPROW[asset->height];
		for(i = 0; i < asset->height; i++)
		{
			row_pointers[i] = &raw_data[i * asset->width * color_channels];
		}

// read the image
		while(jpeg_decompress.output_scanline < jpeg_decompress.output_height)
		{
			jpeg_read_scanlines(&jpeg_decompress, 
				&row_pointers[jpeg_decompress.output_scanline], 
				jpeg_decompress.output_height - jpeg_decompress.output_scanline);
		}

// close the file
		jpeg_finish_decompress(&jpeg_decompress);
		jpeg_destroy((j_common_ptr)&jpeg_decompress);
		fclose(stream);

// convert to a VFrame
		data = new VFrame(0, 
					asset->width,
					asset->height);

		switch(color_channels)
		{
			case 1:
				raw_to_frame(raw_data, data, (int)0, 0, asset->width, asset->height,
					0, 0, asset->width, asset->height, VMAX, 0, use_float, FILEBASE_GREY);
				break;

			case 3:
				raw_to_frame(raw_data, data, 0, 0, asset->width, asset->height,
					0, 0, asset->width, asset->height, VMAX, 0, use_float, FILEBASE_RAW);
				break;

			default:
				printf("FileJPEG::read_raw unrecognized color depth %d.\n", color_channels);
				break;
		};

// delete temporary buffers
		delete [] row_pointers;
		delete [] raw_data;
	}
	return 0;
return 0;
}
