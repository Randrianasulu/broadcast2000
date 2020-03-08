#include "assets.h"
#include "binary.h"
#include "file.h"
#include "filepcm.h"
#include "pluginbuffer.h"
#include <stdlib.h>
#include <string.h>

FilePCM::FilePCM(Asset *asset, File *file) : FileBase(asset, file) 
{
	if(asset->format == 0) asset->format = PCM;
}

FilePCM::~FilePCM()
{
}

int FilePCM::reset_parameters_derived()
{
return 0;
}

int FilePCM::open_file(int rd, int wr)
{
	char flags[10];
	this->wr = wr;
	this->rd = rd;


	get_mode(flags, rd, wr);

	if(!(stream = fopen(asset->path, flags))) 
	{
		perror("FilePCM::open_file");
		return 1;
	}

// skip header for write
	if(wr)
	{
  		long position = ftell(stream);
  		fseek(stream, 0, SEEK_END);
  		long length = ftell(stream);
		fseek(stream, position, SEEK_SET);
		
		if(length < asset->header) for(int i = 0; i < asset->header; i++) fputc(0, stream);
	}
	else
	if(rd)
	{
		fseek(stream, asset->header, SEEK_SET);
	}

	internal_byte_order = get_byte_order();
	reset_parameters();
	return 0;
return 0;
}

int FilePCM::close_file_derived()
{
	if(asset->format == WAV && wr)
	{
		write_header();
	}

	fclose(stream);
return 0;
}

int FilePCM::set_channel(int channel) { this->audio_channel = channel; return 0;
}

long FilePCM::get_audio_length()
{
	long result, position;

	switch(asset->bits)
	{
		case BITSIMA4:
// We use byte counts in case user wants a headerless IMA4 file
			position = ftell(stream);
			fseek(stream, 0, SEEK_END);
			result = ftell(stream);
			fseek(stream, position, SEEK_SET);
			result -= asset->header;
			
			result = ima4_bytes_to_samples(result, asset->channels);
			break;

		default:
			position = ftell(stream);

			fseek(stream, 0, SEEK_END);
			result = ftell(stream);
			fseek(stream, position, SEEK_SET);
			result -= asset->header;

			if(result && asset->channels && asset->bits)
				result = result / asset->channels / (file->bytes_per_sample(asset->bits));
			else
				result = 0;
			break;
	}
	return result;
}

long FilePCM::get_audio_position()
{
	long result;
	switch(asset->bits)
	{
		case BITSIMA4:
			result = ftell(stream) - asset->header;
			result = ima4_bytes_to_samples(result, asset->channels);
			break;
		
		defaults:
			result = ftell(stream) - asset->header;

			if(result)
			result = result / asset->channels / (file->bytes_per_sample(asset->bits));
			else
			result = 0;
			break;
	}
	return result;
}

long FilePCM::get_memory_usage()
{
// give buffer length plus padding
	return prev_len * asset->channels * (file->bytes_per_sample(asset->bits)) + 256;
}

int FilePCM::set_audio_position(long x)
{
	switch(asset->bits)
	{
		case BITSIMA4:
			
			break;
		
		default:
			fseek(stream, x * asset->channels * (file->bytes_per_sample(asset->bits)) + asset->header, SEEK_SET);
			break;
	}
	return 0;
return 0;
}

int FilePCM::seek_end()
{
	fseek(stream, 0, SEEK_END);
	audio_position = get_audio_length();
return 0;
}

int FilePCM::seek_start()
{
	fseek(stream, asset->header, SEEK_SET);
	audio_position = 0;
return 0;
}


int FilePCM::write_samples(float **buffer, 
		PluginBuffer *audio_ram, 
		long byte_offset, 
		long allocated_samples, 
		long len)
{
	int result;
	long bytes;

//printf("FilePCM::write_samples 1\n");
	get_audio_buffer(&audio_buffer_out, len, asset->bits, asset->channels);
//printf("FilePCM::write_samples 2\n");
	bytes = samples_to_raw(audio_buffer_out, 
					buffer, 
					len, 
					asset->bits, 
					asset->channels,
					asset->byte_order,
					asset->signed_);
//printf("FilePCM::write_samples 3\n");

	if(!fwrite(audio_buffer_out, bytes, 1, stream)) 
	{
		perror("FilePCM::write_samples");
		result = 1;
	}
	else
	{
		result = 0;
	}

	return result;
return 0;
}

int FilePCM::read_samples(PluginBuffer *shared_buffer, long offset, long len, int feather, 
	long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	float *buffer = (float*)shared_buffer->get_data() + offset;
	get_audio_buffer(&audio_buffer_in, len, asset->bits, asset->channels);

	if(read_raw(audio_buffer_in, len))
	{
//		perror("FilePCM::read_samples");
//		return 1;
	}

	return raw_to_samples(buffer, 
				audio_buffer_in, 
				len, 
				asset->bits, 
				asset->channels, 
				audio_channel, 
				feather, 
				lfeather_len, 
				lfeather_gain, 
				lfeather_slope);
return 0;
}






// ============================== PCM file routines

int FilePCM::write_header()
{
	char frame = asset->channels * (file->bytes_per_sample(asset->bits));
	long position = ftell(stream);
	fseek(stream, 0, SEEK_END);
	long filelength = ftell(stream);

	fseek(stream, 0, SEEK_SET);
	fprintf(stream, "RIFF");
	putfourswap(filelength - 8, stream);     // length difference
	fprintf(stream, "WAVEfmt ");
	fputc(16, stream);
	fputc(0, stream);
	fputc(0, stream);
	fputc(0, stream);
	fputc(1, stream);
	fputc(0, stream);
	fputc(asset->channels, stream);        // channels
	fputc(0, stream);
	putfourswap(asset->rate, stream);             // rate
	putfourswap(asset->rate * frame, stream);     // bytes per second
	fputc(frame, stream);                  // frame
	fputc(0, stream);
	fputc(file->bytes_per_sample(asset->bits) * 8, stream); // bits
	fputc(0, stream);
	fprintf(stream, "data");
	putfourswap(filelength - 44, stream);   // length difference

	fseek(stream, position, SEEK_SET);
	return 0;
return 0;
}

int FilePCM::read_header()
{
	long position = ftell(stream);

	fseek(stream, 22, SEEK_SET);
	asset->channels = fgetc(stream);

	fseek(stream, 24, SEEK_SET);
	asset->rate = getfourswap(stream);

	fseek(stream, 34, SEEK_SET);
	asset->bits = fgetc(stream);
  
	fseek(stream, position, SEEK_SET);

	asset->format = WAV;
	asset->header = 44;
	asset->byte_order = 1;
	if(asset->bits == 8) asset->signed_ = 0;  else asset->signed_ = 1;
	return 0;
return 0;
}

int FilePCM::read_raw(char *buffer, long len)
{
	int result;

// only load if position has changed or this is the first buffer
	if(prev_buffer_position == -1 || 
		prev_buffer_position != ftell(stream) ||
		prev_len != len)
	{
		long bytes;
		bytes = len * asset->channels * (file->bytes_per_sample(asset->bits));

		prev_buffer_position = ftell(stream);
		prev_len = len;

		result = fread(buffer, bytes, 1, stream);
		if(!result) return 1;

		if(asset->bits == 8 && !asset->signed_)       // fix the unsigned numbers to signed
		{    // change unsigned to signed
			int i = 0, len_ = bytes - 4;
			while(i < len_)
			{
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
			}
			while(i < bytes)
			{
				buffer[i++] ^= 128;
			}
		}
		if(!internal_byte_order && asset->byte_order || internal_byte_order && !asset->byte_order)
		{            // swap bytes if not intel
			swap_bytes(file->bytes_per_sample(asset->bits), (unsigned char*)buffer, bytes);
		}		
	}

	return 0;
return 0;
}




FileWAV::FileWAV(Asset *asset, File *file)
 : FilePCM(asset, file)
{
	asset->audio_data = 1;
	asset->format = WAV;
	asset->signed_ = 0;
	asset->header = 44;
	asset->byte_order = 1;
}

FileWAV::~FileWAV()
{
}
