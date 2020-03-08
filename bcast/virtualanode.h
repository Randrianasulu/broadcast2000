#ifndef VIRTUALANODE_H
#define VIRTUALANODE_H


#include "maxbuffers.h"
#include "maxchannels.h"
#include "virtualnode.h"

class VirtualANode : public VirtualNode
{
public:
	VirtualANode(MainWindow *mwindow, 
		Module *real_module, 
		Plugin *real_plugin,
		Transition *real_transition, 
		Patch *patch, 
		Track *track, 
		VirtualNode *parent_module, 
		PluginBuffer **input,
		PluginBuffer **output,
		long buffer_size,
		long fragment_size, 
		int double_buffers, 
		int input_is_master,
		int output_is_master,
		int in,
		int out,
		int out_channels);

	~VirtualANode();

	int create_buffer_ptrs();
	int create_output_buffer();
	int create_input_buffer();
	int delete_input_buffer();
	int delete_output_buffer();
	int delete_buffer_ptrs();

	VirtualNode* create_plugin(Plugin *real_plugin, Transition *real_transition);
	VirtualNode* create_module(Plugin *real_plugin, 
								Module *real_module, 
								Transition *real_transition, 
								VirtualNode *parent_module, 
								Patch *patch, 
								Track *track);

// need *arender for peak updating
	int render(float **audio_out, 
				long audio_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				long source_length, 
				int reverse,
				ARender *arender);

// convenience pointers for PluginBuffers
	float *input_ptr[MAX_BUFFERS];
	float *output_ptr[MAX_BUFFERS];

private:
// need *arender for peak updating
	int render_as_module(float **audio_out, 
				long audio_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				int reverse,
				ARender *arender);

	int render_float_buffer(float *input,         // start of input fragment
				float *output,        // start of output fragment
				long buffer_len,      // fragment length in input scale
				long input_position,  // start of input fragment in project if forward / end of input fragment if reverse
				float constant,       // default scalar in DB
				float minvalue,       // minimum value for automating constant
				int automate,         // 1 for automation enabled
				Autos *autos,    // link list of autos
				Auto **before,
				Auto **after,
				int additive,
				int use_db);

	float* get_module_input(int double_buffer, long fragment_position);
	float* get_module_output(int double_buffer, long fragment_position);

	DB db;
	int out_channels;
	Auto *pan_before[MAXCHANNELS], *pan_after[MAXCHANNELS];
};


#endif
