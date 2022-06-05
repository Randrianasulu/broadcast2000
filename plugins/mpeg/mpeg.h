#ifndef MPEG_H
#define MPEG_H

// the simplest I/O plugin possible

#include "bcbase.h"
#include "libmpeg3/libmpeg3.h"
#include "mpegwindow.inc"
#include "overlayframe.inc"
#include "pluginioclient.h"

class VideoEncoder
{
public:
	VideoEncoder();
	~VideoEncoder();
	
	int start();
	
	SharedMem *buffer;
	int pid;
};

class AudioEncoder
{
public:
	AudioEncoder();
	~AudioEncoder();
	
	int start();
	
	SharedMem *buffer;
	int pid;
};

class MpegMain : public PluginIOClient
{
public:
	MpegMain(int argc, char *argv[]);
	~MpegMain();

	int initialize();

// required by the fileio plugin client
	int plugin_is_audio();              // plugin supports audio/default = 0
	int plugin_is_video();              // plugin supports video/default = 0
	const char* plugin_title();       // The title is the item appearing in format popups and assets
	int get_audio_parameters();   // Get parameters only for writing
	int get_video_parameters();   // Get parameters only for writing
	int interrupt_aparameters();
	int interrupt_vparameters();
	int check_header(char *path);   // Check to see if the file is of the right type.  Return 1 if it is.
	int open_file(char *path, int rd, int wr);
	int close_file();
	long get_alength();
	long get_vlength();
	int seek_end();
	int seek_start();
	long get_aposition();
	long get_vposition();
	int set_aposition(long sample);
	int set_vposition(long frame);
	int set_channel(int channel);
	int set_layer(int layer);
	int write_samples(float **buffer, long len);
	int write_frame(VFrame *frame, int use_alpha, int use_float);
	int read_samples(float *buffer, long len, 
			int feather, 
			long lfeather_len, float lfeather_gain, float lfeather_slope);
	int read_frame_ptr(int use_alpha, 
			int use_float, 
			long &shared_id, 
			long &shared_size);
	int read_frame(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);
	int read_raw_frame(VFrame *frame);
	int read_raw_frame_possible();
	int test_strategy(int strategy);

// Defaults are how the encoding parameters get propogated from the GUI
	int load_defaults();
	int save_adefaults();
	int save_vdefaults();

	int abitrate, vbitrate;
	int interlaced;
	int video_layer;

private:
	int reset_parameters();
	int read_header();
	int read_rgb24_frame(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate);    // Read a frame into the rgb24 buffer from the libmpeg3 library

	int current_channel;
	int current_layer;
	long current_sample, seek_sample;
	long current_frame, seek_frame;
	Defaults *adefaults, *vdefaults;
	mpeg3_t *mpeg;       // File handle
	float *audio_buffer;        // Output buffer for file
	long audio_allocated;
	unsigned char *rgb24_frame, **rgb24_rows;     // Output buffer for file
	VFrame *temp_frame;      // Frame for overlaying
	PluginBuffer *temp_frame_buffer;   // Buffer for temp frame
	OverlayFrame *overlayer;

// Encoding parameters
	MpegAudioThread *athread;
	MpegVideoThread *vthread;
// Temp filenames
	char **temp_audio;
	char **temp_video;
	char path[1024];
	AudioEncoder *audio_encoder;
	VideoEncoder *video_encoder;
};

#endif
