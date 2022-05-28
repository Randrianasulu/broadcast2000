#ifndef PLUGINACLIENT_H
#define PLUGINACLIENT_H



#include "maxbuffers.h"
#include "pluginclient.h"

class PluginAClient : public PluginClient
{
public:
	PluginAClient(int argc, char *argv[]);
	~PluginAClient();
	
	int create_buffer_ptrs();
	int delete_buffer_ptrs();
	int get_render_ptrs();
	int init_realtime_parameters();
	int process_realtime(long size);
// realtime process for a multichannel plugin
	virtual int process_realtime(long size, float **input_ptr, float **output_ptr) {return 0;};
// realtime process for a single channel plugin
	virtual int process_realtime(long size, float *input_ptr, float *output_ptr) {return 0;};
	int plugin_is_audio();

// point to the start of the buffers
	ArrayList<float**> input_ptr_master;
	ArrayList<float**> output_ptr_master;
// point to the regions for a single render
	float **input_ptr_render;
	float **output_ptr_render;
	int project_sample_rate;      // sample rate of incomming data
};



#endif
