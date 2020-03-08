#include <string.h>
#include "assets.h"
#include "file.h"
#include "filejpeglist.h"
#include "jpegwrapper.h"
#include "quicktime.h"
#include "vframe.h"

#include <ctype.h>
#include <stdlib.h>


FileJPEGList::FileJPEGList(Asset *asset, File *file) : FileBase(asset, file)
{
	reset_parameters();
	asset->video_data = 1;
	asset->format = JPEG_LIST;
	reset_parameters_derived();
}

FileJPEGList::~FileJPEGList()
{
	close_file();
}

int FileJPEGList::reset_parameters_derived()
{
	data = 0;
	stream = 0;
return 0;
}

int FileJPEGList::open_file(int rd, int wr)
{
	char flags[10];
	this->rd = rd;
	this->wr = wr;
	get_mode(flags, rd, wr);

	if(!(stream = fopen(asset->path, flags))) 
	{
		perror("FileJPEGList::open_file");
		return 1;
	}

// skip header for write
	if(wr)
	{
	}
	else
	if(rd)
	{
		return read_header();
	}

	video_position = 0;

	return 0;
return 0;
}

int FileJPEGList::can_copy_from(Asset *asset)
{
	if(asset->format == JPEG_LIST)
		return 1;
	else
	if(asset->format == MOV && match4(asset->compression, QUICKTIME_JPEG))
		return 1;
	
	return 0;
return 0;
}

long FileJPEGList::get_video_length()
{
	return path_list.total;
}

int FileJPEGList::seek_end()
{
	video_position = path_list.total;
return 0;
}

int FileJPEGList::seek_start()
{
	video_position = 0;	
return 0;
}

int FileJPEGList::set_video_position(long x)
{
	video_position = x;
return 0;
}

long FileJPEGList::get_memory_usage()
{
	return 1;
}

int FileJPEGList::set_layer(int layer)
{
	this->video_layer = layer;
return 0;
}


int FileJPEGList::close_file_derived()
{
	if(stream)
	{
		if(wr) write_header();
		fclose(stream);
		for(int i = 0; i < path_list.total; i++) delete [] path_list.values[i];
		path_list.remove_all();
	}
	if(data) delete data;
	reset_parameters();
	return 0;
return 0;
}

int FileJPEGList::write_header()
{
	fprintf(stream, "JPEGLIST\n");
	fprintf(stream, "# First line is always JPEGLIST\n");
	fprintf(stream, "# Second line is frame rate\n");
	fprintf(stream, "%f\n", asset->frame_rate);
	fprintf(stream, "# Width\n");
	fprintf(stream, "%d\n", asset->width);
	fprintf(stream, "# Height\n");
	fprintf(stream, "%d\n", asset->height);
	fprintf(stream, "# List of JPEG images follows\n");
	for(int i = 0; i < path_list.total; i++)
	{
		fprintf(stream, "%s\n", path_list.values[i]);
	}
return 0;
}

int FileJPEGList::read_header()
{
	char string[1024], *new_entry;

// Don't want a user configured frame rate to get destroyed
// 	if(asset->frame_rate == 0)
// 		asset->frame_rate = 10;
	asset->width = 256;
	asset->height = 256;
	fseek(stream, 0, SEEK_SET);

// Get information about the frames
	do
	{
		fgets(string, 1024, stream);
	}while(!feof(stream) && (string[0] == '#' || string[0] == ' ' || isalpha(string[0])));

// Don't want a user configured frame rate to get destroyed
	if(asset->frame_rate == 0)
		asset->frame_rate = atof(string);

	do
	{
		fgets(string, 1024, stream);
	}while(!feof(stream) && (string[0] == '#' || string[0] == ' '));
	asset->width = atol(string);

	do
	{
		fgets(string, 1024, stream);
	}while(!feof(stream) && (string[0] == '#' || string[0] == ' '));
	asset->height = atol(string);

	asset->layers = 1;
	asset->audio_data = 0;
	asset->video_data = 1;

// Get all the paths
	while(!feof(stream))
	{
		fgets(string, 1024, stream);
		if(strlen(string) && string[0] != '#' && string[0] != ' ' && !feof(stream))
		{
			string[strlen(string) - 1] = 0;
			path_list.append(new_entry = new char[strlen(string) + 1]);
			strcpy(new_entry, string);
		}
	}
//for(int i = 0; i < path_list.total; i++) printf("%s\n", path_list.values[i]);
	return 0;
return 0;
}

int FileJPEGList::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
// forces read directly into frame
	read_raw(frame, in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, alpha, use_alpha, 
		use_float, interpolate);
	video_position++;
return 0;
}

VFrame* FileJPEGList::read_frame(int use_alpha, int use_float)
{
	read_raw(0, 0, 0, (float)asset->width, (float)asset->height, 
		0, 0, (float)asset->width, (float)asset->height, VMAX, 0, use_float, 0);

	video_position++;
	return data;
}


long FileJPEGList::compressed_frame_size()
{
	FILE *jpeg_in;
	long result;

	if(!(jpeg_in = fopen(path_list.values[video_position], "rb")))
	{
		printf("FileJPEGList::compressed_frame_size %s", path_list.values[video_position]);
		return 0;
	}

	fseek(jpeg_in, 0, SEEK_END);
	result = ftell(jpeg_in);
	fclose(jpeg_in);
	return result;
}

int FileJPEGList::read_compressed_frame(VFrame *buffer)
{
	FILE *jpeg_in;
	long result, length;

	if(!(jpeg_in = fopen(path_list.values[video_position], "rb")))
	{
		printf("FileJPEGList::read_compressed_frame %s", path_list.values[video_position]);
		return 1;
	}

	fseek(jpeg_in, 0, SEEK_END);
	length = ftell(jpeg_in);
	fseek(jpeg_in, 0, SEEK_SET);

	if(buffer->get_compressed_allocated() < length) 
	{
		fclose(jpeg_in);
		return 1;
	}
	result = !fread(buffer->get_data(), length, 1, jpeg_in);
	buffer->set_compressed_size(length);
	fclose(jpeg_in);
	video_position++;
	return result;
return 0;
}

int FileJPEGList::write_compressed_frame(VFrame *buffer)
{
	char *path;
	int result = 0;
	FILE *file;

	path = create_path();
	if(!(file = fopen(path, "wb")))
	{
		perror("FileJPEGList::write_compressed_frame\n");
		result = 1;
	}
	else
	{
		result = !fwrite(buffer->get_data(), buffer->get_compressed_size(), 1, file);
		fclose(file);
		video_position++;
	}
	return result;
return 0;
}


int FileJPEGList::read_raw(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	if(!frame && !data)
	{
// user wants frame stored in file handler's frame but data hasn't been created yet.
		frame = data = new VFrame(0, asset->width, asset->height);
	}

	if(!frame)
	{
// user wants frame stored in file handler's frame and data has been created.
		frame = data;
	}

	prev_layer = 0;
	prev_frame_position = video_position;

	if(video_position >= path_list.total) return 1;

	get_video_buffer(&video_buffer_in, 24);
	get_row_pointers(video_buffer_in, &row_pointers_in, 24);


// read the raw data
	if(video_position >= 0 && video_position < path_list.total)
	{
		FILE *jpeg_in;
		struct jpeg_decompress_struct jpeg_decompress;
		struct jpeg_error_mgr jpeg_error;
		JSAMPROW row_pointer[1];
		int i;
		int color_channels;

		if(!(jpeg_in = fopen(path_list.values[video_position], "rb")))
		{
			printf("FileJPEGList::jpeg_read_header %s", path_list.values[video_position]);
			return 1;
		}

		jpeg_decompress.err = jpeg_std_error(&jpeg_error);
		jpeg_create_decompress(&jpeg_decompress);

		jpeg_stdio_src(&jpeg_decompress, jpeg_in);
		jpeg_read_header(&jpeg_decompress, TRUE);

		if(asset->width < jpeg_decompress.image_width ||
			asset->height < jpeg_decompress.image_height)
		{
			jpeg_destroy((j_common_ptr)&jpeg_decompress);
			fclose(jpeg_in);
			return 1;
		}

		jpeg_start_decompress(&jpeg_decompress);
		color_channels = jpeg_decompress.jpeg_color_space;
		if(use_float) jpeg_decompress.dct_method = JDCT_FLOAT;

// read the image
		while(jpeg_decompress.output_scanline < jpeg_decompress.output_height &&
			jpeg_decompress.output_scanline < asset->height)
		{
			jpeg_read_scanlines(&jpeg_decompress, 
				(JSAMPROW*)&row_pointers_in[jpeg_decompress.output_scanline], 
				jpeg_decompress.output_height - jpeg_decompress.output_scanline);
		}

// close the file
		jpeg_finish_decompress(&jpeg_decompress);
		jpeg_destroy((j_common_ptr)&jpeg_decompress);
		fclose(jpeg_in);

		switch(color_channels)
		{
			case 1:
				raw_to_frame(video_buffer_in, frame, 
					in_x1, in_y1, in_x2, in_y2,
					out_x1, out_y1, out_x2, out_y2, alpha, 
					use_alpha, use_float, FILEBASE_RAW, interpolate, asset->width, asset->height);
				break;

			case 3:
				raw_to_frame(video_buffer_in, frame, 
					in_x1, in_y1, in_x2, in_y2,
					out_x1, out_y1, out_x2, out_y2, alpha, 
					use_alpha, use_float, OVERLAY_RGB, interpolate, asset->width, asset->height);
				break;

			default:
				printf("FileJPEGList::read_raw unrecognized color depth %d.\n", color_channels);
				break;
		}
	}
	return 0;
return 0;
}

char* FileJPEGList::create_path()
{
	int k;
	char *path;
	char output[1024];
	if(video_position >= path_list.total)
	{
		strcpy(output, asset->path);
		for(k = strlen(output) - 1; k > 0 && output[k] != '.'; k--)
			;
		if(k <= 0) k = strlen(output);
		sprintf(&output[k], "%06d.jpg", video_position);
		path = new char[strlen(output) + 1];
		strcpy(path, output);
		path_list.append(path);
		return path;
	}
	else
	{
// Overwrite an old path
		path = path_list.values[video_position];
		return path;
	}
}

int FileJPEGList::write_frames(VFrame ***frames, 
		PluginBuffer *video_ram, 
		int len, 
		int use_alpha, 
		int use_float)
{
	int j, result = 0;
	char *path, string[1024];

// Copy a VFrame using alpha into a temp frame without alpha
	if(frames[0][0]->get_color_model() == VFRAME_VPIXEL)
	{
		get_video_buffer(&video_buffer_out, 24);
		get_row_pointers(video_buffer_out, &row_pointers_out, 24);
	}

// Write one layer only
	for(j = 0; j < len && !result; j++)
	{
		struct jpeg_compress_struct jpeg_compress;
		struct jpeg_error_mgr jpeg_error;
		JSAMPROW row_pointer[1];
		FILE *file;

		if(frames[0][j]->get_color_model() == VFRAME_VPIXEL)
		{
// Copy the VFrame to a raw frame
			frame_to_raw(video_buffer_out, frames[0][j], asset->width, asset->height, use_alpha, use_float, FILEBASE_RAW);
		}
		else
		{
// Get row pointers for a video captured frame
			if(row_pointers_out) delete row_pointers_out;
			row_pointers_out = 0;
			get_row_pointers(frames[0][j]->get_data(), &row_pointers_out, 24);
		}

		path = create_path();

		if(!(file = fopen(path, "wb"))) result = 1;
		else
		{
			jpeg_compress.err = jpeg_std_error(&jpeg_error);
			jpeg_create_compress(&jpeg_compress);
			jpeg_stdio_dest(&jpeg_compress, file);
			jpeg_compress.image_width = asset->width;
			jpeg_compress.image_height = asset->height;
			jpeg_compress.input_components = 3;
			jpeg_compress.in_color_space = JCS_RGB;
			jpeg_set_defaults(&jpeg_compress);
			jpeg_set_quality(&jpeg_compress, asset->quality, 0);
			if(use_float) jpeg_compress.dct_method = JDCT_FLOAT;
			jpeg_start_compress(&jpeg_compress, TRUE);
			while(jpeg_compress.next_scanline < jpeg_compress.image_height)
			{
				row_pointer[0] = row_pointers_out[jpeg_compress.next_scanline];
				jpeg_write_scanlines(&jpeg_compress, row_pointer, 1);
			}
			jpeg_finish_compress(&jpeg_compress);
			jpeg_destroy((j_common_ptr)&jpeg_compress);
			fclose(file);
		}

		video_position++;
	}
	return result;
return 0;
}
