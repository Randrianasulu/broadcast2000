#ifndef VIRTUALVNODE_H
#define VIRTUALVNODE_H

#include "bezierauto.inc"
#include "overlayframe.inc"
#include "vframe.inc"
#include "virtualnode.h"
#include "vrender.inc"

class VirtualVNode : public VirtualNode
{
public:
// construct as a module or a plugin
	VirtualVNode(MainWindow *mwindow, 
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
		int frame_w,
		int frame_h);

	~VirtualVNode();

// expansions
	int create_buffer_ptrs();
	int create_output_buffer();
	int create_input_buffer();
	int delete_input_buffer();
	int delete_output_buffer();
	int delete_output_ptrs();
	int delete_buffer_ptrs();

	VirtualNode* create_plugin(Plugin *real_plugin, Transition *real_transition);
	VirtualNode* create_module(Plugin *real_plugin, 
								Module *real_module, 
								Transition *real_transition, 
								VirtualNode *parent_module, 
								Patch *patch, 
								Track *track);

	int render(VFrame **video_out, 
				long video_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				long source_len, 
				int reverse,
				VRender *vrender,
				float step);


// convenience pointers to frames in the shared memory
	int frame_w, frame_h;     // dimensions of frame
	VFrame **output_ptr[MAX_BUFFERS];
	VFrame **input_ptr[MAX_BUFFERS];  


private:
	int render_as_module(VFrame **video_out, 
				long video_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				int reverse,
				float step,
				VRender *vrender);

	int render_projector(VFrame **input,         // start of input fragment
				VFrame **output,        // start of output fragment
				long buffer_len,      // fragment length in input scale
				long input_position,  // start of input fragment in project if forward / end of input fragment if reverse
				int automate,         // 1 for automation enabled
				float step);

	int render_fade(VFrame **input,         // start of input fragment
			VFrame **output,        // start of output fragment
			long buffer_len,      // fragment length in input scale
			long input_position,  // start of input fragment in project if forward / end of input fragment if reverse
			float constant,       // default opacity
			int automate,
			Autos *autos,
			float step);         // 1 for automation enabled

// overlay on the frame with scaling
// Alpha values are from 0 to VMAX
	int transfer_from(VFrame *frame_out, VFrame *frame_in, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate, 
		int mode);

	BezierAuto *projector_before, *projector_after;
	VFrame** get_module_input(int double_buffer, long fragment_position);
	VFrame** get_module_output(int double_buffer, long fragment_position);
	VFrame** get_buffer_ptr(VPixel *raw_data, long frames, long frame_w, long frame_h);
	int delete_buffer_ptr(VFrame** buffer_ptr, long frames);
	OverlayFrame *overlayer;
};


#endif
