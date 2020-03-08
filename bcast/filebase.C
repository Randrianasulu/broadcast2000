#include <string.h>
#include "assets.h"
#include "byteorder.h"
#include "file.h"
#include "filebase.h"
#include "overlayframe.h"
#include "sizes.h"

#include <stdlib.h>

FileBase::FileBase(Asset *asset, File *file)
{
	this->file = file;
	this->asset = asset;
	internal_byte_order = get_byte_order();
	init_ima4();
	reset_parameters();
	overlayer = 0;
}

FileBase::~FileBase()
{
	if(audio_buffer_in) delete [] audio_buffer_in;
	if(audio_buffer_out) delete [] audio_buffer_out;
	if(video_buffer_in) delete [] video_buffer_in;
	if(video_buffer_out) delete [] video_buffer_out;
	if(row_pointers_in) delete [] row_pointers_in;
	if(row_pointers_out) delete [] row_pointers_out;
	if(float_buffer) delete [] float_buffer;
	if(overlayer) delete overlayer;
	delete_ima4();
}

int FileBase::close_file()
{
	if(audio_buffer_in) delete [] audio_buffer_in;
	if(audio_buffer_out) delete [] audio_buffer_out;
	if(video_buffer_in) delete [] video_buffer_in;
	if(video_buffer_out) delete [] video_buffer_out;
	if(row_pointers_in) delete [] row_pointers_in;
	if(row_pointers_out) delete [] row_pointers_out;
	if(float_buffer) delete [] float_buffer;
	close_file_derived();
	reset_parameters();
	delete_ima4();
return 0;
}

int FileBase::set_dither()
{
	dither = 1;
return 0;
}

int FileBase::reset_parameters()
{
	dither = 0;
	audio_buffer_in = 0;
	video_buffer_in = 0;
	audio_buffer_out = 0;
	video_buffer_out = 0;
	float_buffer = 0;
	row_pointers_in = 0;
	row_pointers_out = 0;
	prev_buffer_position = -1;
	prev_frame_position = -1;
	prev_len = 0;
	prev_bytes = 0;
	prev_track = -1;
	prev_layer = -1;
	audio_channel = 0;
	video_layer = 0;
	ulawtofloat_table = 0;
	floattoulaw_table = 0;
	audio_position = 0;
	video_position = 0;

	delete_ulaw_tables();
	reset_parameters_derived();
return 0;
}

int FileBase::get_mode(char *mode, int rd, int wr)
{
	if(rd && !wr) sprintf(mode, "rb");
	else
	if(!rd && wr) sprintf(mode, "wb");
	else
	if(rd && wr)
	{
		int exists = 0;
		FILE *stream;

		if(stream = fopen(asset->path, "rb")) 
		{
			exists = 1; 
			fclose(stream); 
		}

		if(exists) sprintf(mode, "rb+");
		else
		sprintf(mode, "wb+");
	}
return 0;
}










// ======================================= audio codecs

long FileBase::samples_to_raw(char *out_buffer, 
							float **in_buffer,
							long input_len, 
							int bits, 
							int channels,
							int byte_order,
							int signed_)
{
	int output_advance;       // number of bytes in a sample
	float *buffer_channel;    // channel in input buffer
	float *buffer_channel_end;
	int channel;
	long int_sample, int_sample2;
	float float_sample;
	long dither_value, dither_scale = 255;
	long bytes = input_len * channels * (file->bytes_per_sample(bits));
	int machine_byte_order = get_byte_order();

	switch(bits)
	{
		case BITSLINEAR8:
			{
				char *output_ptr, *output_end;
				output_advance = channels;
				for(channel = 0; channel < channels; channel++)
				{
					output_ptr = out_buffer + channel;
					buffer_channel = in_buffer[channel];
					buffer_channel_end = buffer_channel + input_len;

					if(dither)
					{
						for( ; buffer_channel < buffer_channel_end; buffer_channel++)
						{
							float_sample = *buffer_channel * 0x7fff;
							int_sample = (long)float_sample;
							if(int_sample > -0x7f00) { dither_value = rand() % dither_scale; int_sample -= dither_value; }
							int_sample /= 0x100;  // rotating bits screws up the signs
							*output_ptr = int_sample;
							output_ptr += output_advance;
						}
					}
					else
					{
						for( ; buffer_channel < buffer_channel_end; buffer_channel++)
						{
							float_sample = *buffer_channel * 0x7f;
							*output_ptr = (char)float_sample;
							output_ptr += output_advance;
						}
					}
				}

// fix signed
				if(!signed_)
				{
					output_ptr = out_buffer;
					output_end = out_buffer + bytes;

					for( ; output_ptr < output_end; output_ptr++)
						*output_ptr ^= 0x80;
				}
			}
			break;

		case BITSLINEAR16:
			{
				TWO *output_ptr, *output_end;
				output_advance = channels;
				for(channel = 0; channel < channels; channel++)
				{
					output_ptr = (TWO*)out_buffer + channel;
					buffer_channel = in_buffer[channel];
					buffer_channel_end = buffer_channel + input_len;

					if(dither)
					{
						for( ; buffer_channel < buffer_channel_end; buffer_channel++)
						{
							float_sample = *buffer_channel * 0x7fffff;
							int_sample = (long)float_sample;
							if(int_sample > -0x7fff00) { dither_value = rand() % dither_scale; int_sample -= dither_value; }
							int_sample /= 0x100;
							*output_ptr = int_sample;
							output_ptr += output_advance;
						}
					}
					else
					{
						for( ; buffer_channel < buffer_channel_end; buffer_channel++)
						{
							float_sample = *buffer_channel * 0x7fff;
							*output_ptr = (TWO)float_sample;
							output_ptr += output_advance;
						}
					}
				}
			}
			break;

		case BITSLINEAR24:
			{
				char *output_ptr, *output_end;
				output_advance = asset->channels * 3 - 2;
				for(channel = 0; channel < channels; channel++)
				{
					output_ptr = out_buffer + channel * 3;
					buffer_channel = in_buffer[channel];
					buffer_channel_end = buffer_channel + input_len;

// don't bother dithering 24 bits
					for( ; buffer_channel < buffer_channel_end; buffer_channel++)
					{
						float_sample = *buffer_channel * 0x7fffff;
						int_sample = (long)float_sample;
						int_sample2 = int_sample & 0xff0000;
						*output_ptr++ = (int_sample & 0xff);
						int_sample &= 0xff00;
						*output_ptr++ = (int_sample >> 8);
						*output_ptr = (int_sample2 >> 16);
						output_ptr += output_advance;
					}
				}
			}
			break;
		
		case BITSULAW:
			{
				char *output_ptr;
				output_advance = asset->channels;
//printf("FileBase::samples_to_raw 1\n");
				generate_ulaw_tables();
//printf("FileBase::samples_to_raw 2\n");

				for(channel = 0; channel < channels; channel++)
				{
					output_ptr = out_buffer + channel;
					buffer_channel = in_buffer[channel];
					buffer_channel_end = buffer_channel + input_len;
					for( ; buffer_channel < buffer_channel_end; buffer_channel++)
					{
						*output_ptr = floattoulaw(*buffer_channel);
						output_ptr += output_advance;
					}
				}
//printf("FileBase::samples_to_raw 3\n");
			}
			break;
		
		case BITSIMA4:
			{
				generate_ulaw_tables();
			}
			break;
	}

// swap bytes
	if((bits == BITSLINEAR16 && byte_order != machine_byte_order) ||
		(bits == BITSLINEAR24 && !byte_order))
	{
		swap_bytes(file->bytes_per_sample(bits), (unsigned char*)out_buffer, bytes);
	}

	return bytes;
}


#define READ_8_MACRO \
				sample = *inbuffer_8;                   \
				sample /= 0x7f; \
				inbuffer_8 += input_frame;

#define READ_16_MACRO \
				sample = *inbuffer_16;                   \
				sample /= 0x7fff; \
				inbuffer_16 += input_frame;

#define READ_24_MACRO \
				sample = (unsigned char)*inbuffer_24++;  \
				sample_24 = (unsigned char)*inbuffer_24++; \
				sample_24 <<= 8;                           \
				sample += sample_24;                       \
				sample_24 = *inbuffer_24;                  \
				sample_24 <<= 16;                          \
				sample += sample_24;                       \
				sample /= 0x7fffff; \
				inbuffer_24 += input_frame; \

#define READ_ULAW_MACRO \
				sample = ulawtofloat(*inbuffer_8);                   \
				inbuffer_8 += input_frame;

#define LFEATHER_MACRO1 \
				for(feather_current = 0; feather_current < lfeather_len; \
					output_current++, feather_current++) \
				{

#define LFEATHER_MACRO2 \
					current_gain = lfeather_gain + lfeather_slope * feather_current; \
					out_buffer[output_current] = out_buffer[output_current] * (1 - current_gain) + sample * current_gain; \
				}

#define CENTER_MACRO1 \
				for(; output_current < samples; \
					output_current++) \
				{

#define CENTER_MACRO2 \
					out_buffer[output_current] += sample; \
				}

int FileBase::raw_to_samples(float *out_buffer, char *in_buffer, 
		long samples, int bits, int channels, int channel, int feather, 
		float lfeather_len, float lfeather_gain, float lfeather_slope)
{
	long output_current = 0;  // position in output buffer
	long input_len = samples;     // length of input buffer
// The following are floats because they are multiplied by the slope to get the gain.
	float feather_current;     // input position for feather

	float sample; 
	char *inbuffer_8;               // point to actual byte being read
	TWO *inbuffer_16;
	char *inbuffer_24;
	int sample_24;                                         
	float current_gain;
	int input_frame;                   // amount to advance the input buffer pointer

// set up the parameters
	switch(bits)
	{
		case BITSLINEAR8:  
			inbuffer_8 = in_buffer + channel;
			input_frame = channels;
			break;
			
		case BITSLINEAR16: 
			inbuffer_16 = (TWO *)in_buffer + channel;          
			input_frame = channels;
			break;
			 
		case BITSLINEAR24: 
			inbuffer_24 = in_buffer + channel * 3;
			input_frame = channels * file->bytes_per_sample(bits) - 2; 
			break;
		
		case BITSULAW:
			generate_ulaw_tables();
			inbuffer_8 = in_buffer + channel;
			input_frame = channels;
			break;
		
		case BITSIMA4:
			generate_ulaw_tables();
			inbuffer_8 = in_buffer + channel;
			input_frame = channels;
			break;
	}

// read the data
// ================== calculate feathering and add to buffer ================
	if(feather)
	{
// left feather
		switch(bits)
		{
			case BITSLINEAR8:
				LFEATHER_MACRO1;                                             
				READ_8_MACRO; 
				LFEATHER_MACRO2;
				break;

			case BITSLINEAR16:
				LFEATHER_MACRO1;                                             
				READ_16_MACRO; 
				LFEATHER_MACRO2;
				break;

			case BITSLINEAR24:                                               
				LFEATHER_MACRO1;                                             
				READ_24_MACRO; 
				LFEATHER_MACRO2;
				break;
			
			case BITSULAW:
				LFEATHER_MACRO1;
				READ_ULAW_MACRO;
				LFEATHER_MACRO2;
				break;
			
			case BITSIMA4:
				break;
		}
	

// central region
		switch(bits)
		{
			case BITSLINEAR8:                                                  
				CENTER_MACRO1;
				READ_8_MACRO; 
				CENTER_MACRO2;
				break;

			case BITSLINEAR16:
				CENTER_MACRO1;
				READ_16_MACRO;
				CENTER_MACRO2;
				break;

			case BITSLINEAR24:
				CENTER_MACRO1;
				READ_24_MACRO;
				CENTER_MACRO2;
				break;
			
			case BITSULAW:
				CENTER_MACRO1;
				READ_ULAW_MACRO;
				CENTER_MACRO2;
				break;
			
			case BITSIMA4:
				break;
		}
	}
	else
// ====================== don't feather and overwrite buffer =================
	{
		switch(bits)
		{
			case BITSLINEAR8:
				for(; output_current < input_len; 
					output_current++) 
				{ READ_8_MACRO; out_buffer[output_current] = sample; }
				break;

			case BITSLINEAR16:
				for(; output_current < input_len; 
					output_current++) 
				{ READ_16_MACRO; out_buffer[output_current] = sample; }
				break;

			case BITSLINEAR24:
				for(; output_current < input_len; 
					output_current++) 
				{ READ_24_MACRO; out_buffer[output_current] = sample; }
				break;
			
			case BITSULAW:
				for(; output_current < input_len; 
					output_current++) 
				{ READ_ULAW_MACRO; out_buffer[output_current] = sample; }
				break;
			
			case BITSIMA4:
				break;
		}
	}

	return 0;
return 0;
}

int FileBase::overlay_float_buffer(float *out_buffer, float *in_buffer, 
		long samples, 
		float lfeather_len, float lfeather_gain, float lfeather_slope)
{
	long output_current = 0;
	float sample, current_gain;
	float feather_current;     // input position for feather

	LFEATHER_MACRO1
		sample = in_buffer[output_current];
	LFEATHER_MACRO2

	CENTER_MACRO1
		sample = in_buffer[output_current];
	CENTER_MACRO2

	return 0;
return 0;
}


int FileBase::get_audio_buffer(char **buffer, long len, long bits, long channels)
{
	long bytes = len * channels * (file->bytes_per_sample(bits));
	if(*buffer && bytes > prev_bytes) 
	{ 
		delete [] *buffer; 
		*buffer = 0; 
	}
	prev_bytes = bytes;

	if(!*buffer) *buffer = new char[bytes];
return 0;
}

int FileBase::get_float_buffer(float **buffer, long len)
{
	if(*buffer && len > prev_len) 
	{ 
		delete [] *buffer; 
		*buffer = 0; 
	}
	prev_len = len;

	if(!*buffer) *buffer = new float[len];
return 0;
}

int FileBase::get_video_buffer(unsigned char **buffer, int depth)
{
// get a raw video buffer for writing or compression by a library
	if(!*buffer)
	{
// Video compression is entirely done in the library.
		long bytes = asset->width * asset->height * depth;
		*buffer = new unsigned char[bytes];
	}
	return 0;
return 0;
}

int FileBase::get_row_pointers(unsigned char *buffer, unsigned char ***pointers, int depth)
{
// This might be fooled if a new VFrame is created at the same address with a different height.
	if(*pointers && (*pointers)[0] != &buffer[0])
	{
		delete [] *pointers;
		*pointers = 0;
	}

	if(!*pointers)
	{
		*pointers = new unsigned char*[asset->height];
		for(int i = 0; i < asset->height; i++)
		{
			(*pointers)[i] = &buffer[i * asset->width * depth / 8];
		}
	}
return 0;
}

int FileBase::match4(char *in, char *out)
{
	if(in[0] == out[0] &&
		in[1] == out[1] &&
		in[2] == out[2] &&
		in[3] == out[3])
		return 1;
	else
		return 0;
return 0;
}

int FileBase::can_copy_from(Asset *asset)
{
	return 0;
return 0;
}

int FileBase::search_render_strategies(ArrayList<int>* render_strategies, int render_strategy)
{
	int i;
	for(i = 0; i < render_strategies->total; i++)
		if(render_strategies->values[i] == render_strategy) return 1;

	return 0;
return 0;
}
