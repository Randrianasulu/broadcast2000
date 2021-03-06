#include "funcprotos.h"
#include "qtpng.h"

static int delete_codec(quicktime_video_map_t *vtrack)
{
	quicktime_png_codec_t *codec = ((quicktime_codec_t*)vtrack->codec)->priv;
//printf("quicktime_delete_codec_png 1\n");
	if(codec->buffer) free(codec->buffer);
//printf("quicktime_delete_codec_png 1\n");
	free(codec);
//printf("quicktime_delete_codec_png 2\n");
	return 0;
}

void quicktime_set_png(quicktime_t *file, int compression_level)
{
	int i;
	char *compressor;

	for(i = 0; i < file->total_vtracks; i++)
	{
		if(quicktime_match_32(quicktime_video_compressor(file, i), QUICKTIME_PNG))
		{
			quicktime_png_codec_t *codec = ((quicktime_codec_t*)file->vtracks[i].codec)->priv;
			codec->compression_level = compression_level;
		}
	}
}

static void read_function(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	quicktime_png_codec_t *codec = png_get_io_ptr(png_ptr);
	
	if(length + codec->buffer_position <= codec->buffer_size)
	{
		memcpy(data, codec->buffer + codec->buffer_position, length);
		codec->buffer_position += length;
	}
}

static void write_function(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	quicktime_png_codec_t *codec = png_get_io_ptr(png_ptr);

	if(length + codec->buffer_size > codec->buffer_allocated)
	{
		codec->buffer_allocated += length;
		codec->buffer = realloc(codec->buffer, codec->buffer_allocated);
	}
	memcpy(codec->buffer + codec->buffer_size, data, length);
	codec->buffer_size += length;
}

static void flush_function(png_structp png_ptr)
{
	;
}

static int decode(quicktime_t *file, unsigned char **row_pointers, int track)
{
	int result = 0;
	longest i;
	quicktime_video_map_t *vtrack = &(file->vtracks[track]);
	quicktime_trak_t *trak = vtrack->track;
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info = 0;	
	quicktime_png_codec_t *codec = ((quicktime_codec_t*)vtrack->codec)->priv;

	quicktime_set_video_position(file, vtrack->current_position, track);
	codec->buffer_size = quicktime_frame_size(file, vtrack->current_position, track);
	codec->buffer_position = 0;
	if(codec->buffer_size > codec->buffer_allocated)
	{
		codec->buffer_allocated = codec->buffer_size;
		codec->buffer = realloc(codec->buffer, codec->buffer_allocated);
	}

	result = !quicktime_read_data(file, codec->buffer, codec->buffer_size);

	if(!result)
	{
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_set_read_fn(png_ptr, codec, (png_rw_ptr)read_function);
		png_read_info(png_ptr, info_ptr);

/* read the image */
		png_read_image(png_ptr, row_pointers);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	}
	return result;
}

static int encode(quicktime_t *file, unsigned char **row_pointers, int track)
{
	longest offset = quicktime_position(file);
	int result = 0;
	int i;
	quicktime_video_map_t *vtrack = &(file->vtracks[track]);
	quicktime_trak_t *trak = vtrack->track;
	quicktime_png_codec_t *codec = ((quicktime_codec_t*)vtrack->codec)->priv;
	int height = trak->tkhd.track_height;
	int width = trak->tkhd.track_width;
	png_structp png_ptr;
	png_infop info_ptr;
	int depth = quicktime_video_depth(file, track);

	codec->buffer_size = 0;
	codec->buffer_position = 0;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);
	png_set_write_fn(png_ptr,
               codec, 
			   (png_rw_ptr)write_function,
               (png_flush_ptr)flush_function);
	png_set_compression_level(png_ptr, codec->compression_level);
	png_set_IHDR(png_ptr, info_ptr, width, height,
              8, 
			  depth == 24 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA, 
			  PNG_INTERLACE_NONE, 
			  PNG_COMPRESSION_TYPE_DEFAULT, 
			  PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	result = !quicktime_write_data(file, 
				codec->buffer, 
				codec->buffer_size);

printf("quicktime_encode_png %d\n", codec->buffer_size);
	quicktime_update_tables(file,
						file->vtracks[track].track,
						offset,
						file->vtracks[track].current_chunk,
						file->vtracks[track].current_position,
						1,
						codec->buffer_size);

	file->vtracks[track].current_chunk++;
	return result;
}


void quicktime_init_codec_png(quicktime_video_map_t *vtrack)
{
	quicktime_png_codec_t *codec;

/* Init public items */
	((quicktime_codec_t*)vtrack->codec)->priv = calloc(1, sizeof(quicktime_png_codec_t));
	((quicktime_codec_t*)vtrack->codec)->delete_vcodec = delete_codec;
	((quicktime_codec_t*)vtrack->codec)->decode_video = decode;
	((quicktime_codec_t*)vtrack->codec)->encode_video = encode;
	((quicktime_codec_t*)vtrack->codec)->decode_audio = 0;
	((quicktime_codec_t*)vtrack->codec)->encode_audio = 0;

/* Init private items */
	codec = ((quicktime_codec_t*)vtrack->codec)->priv;
	codec->compression_level = 9;
}
