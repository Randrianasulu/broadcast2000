#ifndef PLUGINIOSERVER_H
#define PLUGINIOSERVER_H

#include "assets.inc"
#include "pluginserver.h"

class PluginIOServer : public PluginServer
{
public:
	PluginIOServer();
	PluginIOServer(PluginServer &);
	~PluginIOServer();

// Get parameters for compression	
	int get_audio_parameters();
	int get_video_parameters();
	int interrupt_audio_parameters();
	int interrupt_video_parameters();

	int check_header(char *path);
	int set_cpus(int cpus);
	int open_file(Asset *asset, int rd, int wr);
	int close_file();
	int send_header(Asset *asset);
	int get_header(Asset *asset);	   
// File read commands
	long get_audio_length();
	long get_video_length();
	int seek_end();
	int seek_start();
	long get_video_position();
	long get_audio_position();
	int set_video_position(long frame);
	int set_audio_position(long sample);
	int set_channel(int channel);
	int set_layer(int layer);
	int read_samples(PluginBuffer *buffer, long offset, long len, int feather, 
			long lfeather_len, float lfeather_gain, float lfeather_slope);
	int read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
			float in_x1, float in_y1, float in_x2, float in_y2,
			float out_x1, float out_y1, float out_x2, float out_y2, 
			long alpha, long use_alpha, long use_float, long interpolate);
	VFrame* read_frame(int use_alpha, int use_float);
	int write_frame(VFrame *frame, 
			PluginBuffer *video_ram, 
			long byte_offset, 
			int use_alpha, 
			int use_float);
	int write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len);
	int read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset);
	int read_raw_frame_possible();
	int test_render_strategy(int render_strategy);
};


#endif
