#ifndef PLUGINIOCLIENT_H
#define PLUGINIOCLIENT_H


#include "vframe.inc"
#include "pluginbuffer.inc"

// This is the base class for a plugin that handles IO from a file.


#include "maxbuffers.h"
#include "pluginclient.h"

class PluginIOClient : public PluginClient
{
public:
	PluginIOClient(int argc, char *argv[]);
	virtual ~PluginIOClient();

	int plugin_command_derived(int plugin_command);
	int plugin_open_file();
	int plugin_close_file();

	int plugin_is_fileio();
// Determine whether the file is readable
	int plugin_check_header();
// Get information for writing a new file
	int plugin_get_aparameters();
	int plugin_get_vparameters();
	int plugin_interrupt_aparameters();
	int plugin_interrupt_vparameters();
// Get information for reading a file
	int plugin_set_cpus();
	long plugin_get_alength(); 
	long plugin_get_vlength(); 
	int plugin_seek_end();
	int plugin_seek_start();
	long plugin_get_vposition();
	long plugin_get_aposition();
// Read data
	int plugin_set_vposition();
	int plugin_set_aposition();
	int plugin_set_channel();
	int plugin_set_layer();
	int plugin_read_samples();
	int plugin_read_frame();
	int plugin_read_frame_ptr();
	int plugin_read_raw_frame();
	int plugin_test_strategy();
	int plugin_raw_frame_possible();
	int plugin_write_samples();
	int plugin_write_frame();


// ============================== User entrypoints
	virtual int interrupt_aparameters() { return 0; };
	virtual int interrupt_vparameters() { return 0; };
	virtual long get_alength() { return 0; }; 
	virtual long get_vlength() { return 0; };
	virtual int seek_end() { return 0; };
	virtual int seek_start() { return 0; };
	virtual long get_vposition() { return 0; };
	virtual long get_aposition() { return 0; };
	virtual int set_vposition(long frame) { return 0; };
	virtual int set_aposition(long sample) { return 0; };
	virtual int set_channel(int channel) { return 0; };
	virtual int set_layer(int layer) { return 0; };
	virtual int write_samples(float **buffer, long len) { return 0; };
	virtual int write_frame(VFrame *frame, int use_alpha, int use_float) { return 0; };

// Overlay samples
	virtual int read_samples(float *buffer, long len, 
		int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope) { return 1; };
// Overlay a frame
	virtual int read_frame(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate) { return 1; };
// Read a frame into shared memory and return a pointer to it
	virtual int read_frame_ptr(int use_alpha, 
		int use_float, 
		long &shared_id, 
		long &shared_size) { return 0; };
// Read framebuffer
	virtual int read_raw_frame(VFrame *frame) { return 1; };
	virtual int read_raw_frame_possible() { return 0; };
	virtual int test_strategy(int strategy) { return 0; };


// File parameters which can be filled in by the plugin but default to 0
	int has_audio;
	int sample_rate;
	int channels;
	int bits;
	int byteorder;
	int signed_;
	int header;

	int has_video;
	int layers;
	float frame_rate;
	int width;
	int height;
	int quality;
	char compression[16];
	int cpus;

private:

	int send_header();
	int get_header();

	int project_frame_w;
	int project_frame_h;
};



#endif
