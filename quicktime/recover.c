#include "quicktime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FSEEK fseeko64


#define WIDTH 720
#define HEIGHT 480
#define FRAMERATE (float)24
#define CHANNELS 2
#define SAMPLERATE 48000
#define BITS 16







#define SEARCH_FRAGMENT (longest)0x1000

int main(int argc, char *argv[])
{
	FILE *in;
	quicktime_t *out;
	longest current_byte, ftell_byte;
	longest jpeg_start, jpeg_end;
	longest audio_start, audio_end;
	unsigned char *search_buffer = calloc(1, SEARCH_FRAGMENT);
	unsigned char *copy_buffer = 0;
	int i, found_jfif, found_eoi;
	longest file_size;
	struct stat status;

	if(argc < 3)	   
	{				   
		printf("Recover JPEG and PCM audio from a corrupted movie.\n"
			"Usage: recover <input> <output>\n"
			"Compiled settings:\n"
			"   WIDTH %d\n"
			"   HEIGHT %d\n"
			"   FRAMERATE %.2f\n"
			"   CHANNELS %d\n"
			"   SAMPLERATE %d\n"
			"   BITS %d\n",
			WIDTH,
			HEIGHT,
			FRAMERATE,
			CHANNELS,
			SAMPLERATE,
			BITS);
		exit(1);
	}

	in = fopen(argv[2], "rb");
	if(in)
	{
		printf("Output file exists.\n");
		exit(1);
	}

	in = fopen(argv[1], "rb");
	out = quicktime_open(argv[2], 0, 1);

	if(!in)
	{
		perror("open input");
		exit(1);
	}
	if(!out)
	{
		perror("open output");
		exit(1);
	}

	quicktime_set_audio(out, 
		CHANNELS, 
		SAMPLERATE, 
		BITS, 
		QUICKTIME_TWOS);
	quicktime_set_video(out, 
		1, 
		WIDTH, 
		HEIGHT, 
		FRAMERATE, 
		QUICKTIME_JPEG);
	audio_start = (longest)0x10;
	found_jfif = 0;
	found_eoi = 0;
	ftell_byte = 0;

	if(fstat(fileno(in), &status))
		perror("get_file_length fstat:");
	file_size = status.st_size;
	


	while(ftell_byte < file_size)
	{
// Search forward for JFIF
		current_byte = ftell_byte;
		fread(search_buffer, SEARCH_FRAGMENT, 1, in);
		ftell_byte += SEARCH_FRAGMENT;
		for(i = 0; i < SEARCH_FRAGMENT - 4; i++)
		{
			if(!found_jfif)
			{
				if(search_buffer[i] == 'J' &&
					search_buffer[i + 1] == 'F' &&
					search_buffer[i + 2] == 'I' &&
					search_buffer[i + 3] == 'F')
				{
					current_byte += i - 6;
					FSEEK(in, current_byte, SEEK_SET);
					ftell_byte = current_byte;
					found_jfif = 1;
					audio_end = jpeg_start = current_byte;
					break;
				}
			}
			else
			if(!found_eoi)
			{
				if(search_buffer[i] == 0xff &&
					search_buffer[i + 1] == 0xd9)
				{
					current_byte += i + 2;
					FSEEK(in, current_byte, SEEK_SET);
					ftell_byte = current_byte;
					found_eoi = 1;
					audio_start = jpeg_end = current_byte;
					break;
				}
			}
		}

		if(found_jfif && !found_eoi && audio_end - audio_start > 0)
		{
			long samples = (audio_end - audio_start) / 4;
			copy_buffer = realloc(copy_buffer, audio_end - audio_start);
			FSEEK(in, audio_start, SEEK_SET);
			ftell_byte = audio_start;
			fread(copy_buffer, audio_end - audio_start, 1, in);
			ftell_byte += audio_end - audio_start;
			quicktime_write_audio(out, copy_buffer, samples, 0);
printf("write audio %llx - %llx = %llx\n", audio_end, audio_start, audio_end - audio_start);
			audio_start = audio_end;
		}
		else
		if(found_jfif && found_eoi)
		{
			copy_buffer = realloc(copy_buffer, jpeg_end - jpeg_start);
			FSEEK(in, jpeg_start, SEEK_SET);
			ftell_byte = jpeg_start;
			fread(copy_buffer, jpeg_end - jpeg_start, 1, in);
			ftell_byte += jpeg_end - jpeg_start;
			quicktime_write_frame(out, copy_buffer, jpeg_end - jpeg_start, 0);
			found_jfif = 0;
			found_eoi = 0;
		}
		else
		{
			FSEEK(in, current_byte + SEARCH_FRAGMENT - 4, SEEK_SET);
			ftell_byte = current_byte + SEARCH_FRAGMENT - 4;
		}
	}
printf("\n");

	fclose(in);
	quicktime_close(out);
}




