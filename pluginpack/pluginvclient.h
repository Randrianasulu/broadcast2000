#ifndef PLUGINVCLIENT_H
#define PLUGINVCLIENT_H


#include "maxbuffers.h"
#include "pluginclient.h"
#include "vframe.h"


class PluginVClient : public PluginClient
{
public:
	PluginVClient(int argc, char *argv[]);
	~PluginVClient();

	int create_buffer_ptrs();
	int delete_buffer_ptrs();
	int get_render_ptrs();
	int create_nonrealtime_ptrs();
	int init_realtime_parameters();
	int init_nonrealtime_parameters();
	int delete_nonrealtime_parameters();
	int plugin_is_video();
	int process_realtime(long size);
// realtime process for a multichannel plugin
	virtual int process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr);
// realtime process for a single channel plugin
	virtual int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);

// Non realtime buffer pointers
// Channels of arrays of frames that the client uses.
	VFrame ***video_in, ***video_out;

// point to the start of the buffers
	ArrayList<VFrame***> input_ptr_master;
	ArrayList<VFrame***> output_ptr_master;
// Pointers to the regions for a single render.
// Arrays are channels of arrays of frames.
	VFrame ***input_ptr_render;
	VFrame ***output_ptr_render;

	float project_frame_rate;
	int project_frame_w;
	int project_frame_h;
	int use_float;   // Whether user wants floating point calculations.
	int use_alpha;   // Whether user wants alpha calculations.
	int use_interpolation;   // Whether user wants pixel interpolation.
	float aspect_w, aspect_h;  // Aspect ratio
};



#endif
