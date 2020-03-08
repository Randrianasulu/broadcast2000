#ifndef FILEPLUGIN_H
#define FILEPLUGIN_H

#include "file.inc"
#include "filebase.h"
#include "pluginioserver.inc"

class FilePlugin : public FileBase
{
public:
	FilePlugin(ArrayList<PluginServer*> *plugindb, Asset *asset, File *file);
	~FilePlugin();

	int check_header();
	int open_file(int rd, int wr);
	int close_file_derived();
	long get_video_length();
	long get_audio_length();
	long get_position();
	long get_video_position();
	long get_audio_position();
	long get_memory_usage();
	int set_video_position(long x);
	int set_audio_position(long x);
	int set_channel(int channel);  // set audio channel for reading
	int set_layer(int layer);      // set layer for reading
	int seek_end();
	int seek_start();
	int write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len);
	int write_frames(VFrame ***frames, 
			PluginBuffer *video_ram, 
			int len, 
			int use_alpha, 
			int use_float);
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	VFrame* read_frame(int use_alpha, int use_float);
	int read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset);
	int read_samples(PluginBuffer *buffer, long offset, long len, int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope);

	int read_raw_frame_possible();
	int get_render_strategy(ArrayList<int>* render_strategies);

private:
	int reset_parameters_derived();
	VFrame *data; // frame to return through read_frame
	PluginIOServer *plugin;
	ArrayList<PluginServer*> *plugindb;
};





#endif
