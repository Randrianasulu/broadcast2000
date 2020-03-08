#ifndef FILEPCM_H
#define FILEPCM_H

#include "file.inc"
#include "filebase.h"

class FilePCM : public FileBase
{
public:
	FilePCM(Asset *asset, File *file);
	~FilePCM();

// basic commands for every file interpreter
	int open_file(int rd, int wr);
	int close_file_derived();
	int set_channel(int channel);
	long get_audio_length();
	long get_audio_position();
	long get_memory_usage();
	int set_audio_position(long x);
	int seek_end();
	int seek_start();
	int write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len);
	int read_samples(PluginBuffer *shared_buffer, long offset, long len, int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope);



// PCM file 
	int read_header();     // get format information

	int write_header();
	int read_raw(char *buffer, long len);

	FILE *stream;
private:
	int reset_parameters_derived();
};

class FileWAV : public FilePCM
{
public:
	FileWAV(Asset *asset, File *file);
	~FileWAV();
};


#endif
