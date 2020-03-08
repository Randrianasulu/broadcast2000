#include <string.h>
#include "floatautos.h"
#include "mainwindow.h"
#include "module.h"
#include "overlayframe.h"
#include "patchbay.h"
#include "plugin.h"
#include "pluginbuffer.h"
#include "preferences.h"
#include "transition.h"
#include "vframe.h"
#include "virtualvnode.h"
#include "vmodule.h"
#include "vtrack.h"



VirtualVNode::VirtualVNode(MainWindow *mwindow, 
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
		int frame_h)
 : VirtualNode(mwindow, 
		real_module, 
		real_plugin,
		real_transition, 
		patch, 
		track, 
		parent_module, 
		input,
		output,
		buffer_size,
		fragment_size, 
		double_buffers,
		input_is_master,
		output_is_master,
		in,
		out)
{
	for(int i = 0; i < double_buffers; i++)
	{
		input_ptr[i] = 0;
		output_ptr[i] = 0;
	}
	this->frame_w = frame_w;
	this->frame_h = frame_h;
	projector_before = projector_after = 0;
	overlayer = 0;
}

VirtualVNode::~VirtualVNode()
{
	if(!shared_output) delete_output_buffer();
	if(!shared_input) delete_input_buffer();
	if(overlayer) delete overlayer;
	delete_buffer_ptrs();
}

int VirtualVNode::create_output_buffer()
{
	output[0] = new PluginBuffer(fragment_size * frame_w * frame_h + 1, sizeof(VPixel));
return 0;
}

int VirtualVNode::create_input_buffer()
{
	input[0] = new PluginBuffer(fragment_size * frame_w * frame_h + 1, sizeof(VPixel));
	VFrame **input_ptr = get_buffer_ptr((VPixel*)input[0]->get_data(), fragment_size, frame_w, frame_h);
// zero the input buffer
	for(int i = 0; i < fragment_size; i++)
	{
		input_ptr[i]->clear_frame();
	}
	delete_buffer_ptr(input_ptr, fragment_size);
return 0;
}

int VirtualVNode::delete_input_buffer()
{
	delete input[0];
return 0;
}

int VirtualVNode::delete_output_buffer()
{
	delete output[0];
return 0;
}

int VirtualVNode::create_buffer_ptrs()
{
	int i;
	for(i = 0; i < (input_is_master ? double_buffers : 1); i++)
	{
		input_ptr[i] = get_buffer_ptr((VPixel*)input[i]->get_data(), 
						input_is_master ? buffer_size : fragment_size, 
						frame_w, 
						frame_h);
	}

	for(i = 0; i < (output_is_master ? double_buffers : 1); i++)
	{
		output_ptr[i] = get_buffer_ptr((VPixel*)output[i]->get_data(), 
						output_is_master ? buffer_size : fragment_size, 
						frame_w, 
						frame_h);
	}
return 0;
}

int VirtualVNode::delete_buffer_ptrs()
{
	int i;
	for(i = 0; i < (input_is_master ? double_buffers : 1); i++)
	{
		delete_buffer_ptr(input_ptr[i], input_is_master ? buffer_size : fragment_size);
	}

	for(i = 0; i < (output_is_master ? double_buffers : 1); i++)
	{
		delete_buffer_ptr(output_ptr[i], output_is_master ? buffer_size : fragment_size);
	}
return 0;
}

VFrame** VirtualVNode::get_buffer_ptr(VPixel *raw_data, long frames, long frame_w, long frame_h)
{
	VFrame **result;
	result = new VFrame*[frames];
	for(int i = 0; i < frames; i++)
	{
		result[i] = new VFrame((unsigned char*)(&raw_data[i * frame_w * frame_h]), frame_w, frame_h);
	}
	return result;
}

int VirtualVNode::delete_buffer_ptr(VFrame** buffer_ptr, long frames)
{
	for(int i = 0; i < frames; i++)
	{
		delete buffer_ptr[i];
	}
	delete buffer_ptr;
return 0;
}

VirtualNode* VirtualVNode::create_module(Plugin *real_plugin, 
					Module *real_module, 
					Transition *real_transition, 
					VirtualNode *parent_module, 
					Patch *patch, 
					Track *track)
{
	return new VirtualVNode(mwindow, 
		real_module, 
		0, 
		real_transition, 
		patch, 
		track, 
		this, 
		data_in_input ? input : output,
		output,
		buffer_size,
		fragment_size, 
		double_buffers, 
		input_is_master,
		output_is_master,
		real_transition ? real_transition->in : real_plugin->in,
		real_transition ? real_transition->out : real_plugin->out,
		frame_w,
		frame_h);
}

VirtualNode* VirtualVNode::create_plugin(Plugin *real_plugin, Transition *real_transition)
{
	return new VirtualVNode(mwindow, 
		0, 
		real_plugin, 
		real_transition, 
		0, 
		0, 
		this, 
		data_in_input ? input : output, 
		output,
		buffer_size,
		fragment_size, 
		double_buffers, 
		data_in_input ? input_is_master : output_is_master,
		output_is_master,
		real_transition ? real_transition->in : real_plugin->in,
		real_transition ? real_transition->out : real_plugin->out,
		frame_w,
		frame_h);
}

VFrame** VirtualVNode::get_module_input(int double_buffer, long fragment_position)
{
	VFrame** result;
	if(data_in_input)
	{
		if(input_is_master) 
			result = &input_ptr[double_buffer][fragment_position];
		else
			result = input_ptr[0];
	}
	else
	{
		if(output_is_master)
			result = &output_ptr[double_buffer][fragment_position];
		else
			result = output_ptr[0];
	}
	return result;
}

VFrame** VirtualVNode::get_module_output(int double_buffer, long fragment_position)
{
	VFrame** result;

	if(output_is_master)
		result = &output_ptr[double_buffer][fragment_position];
	else
		result = output_ptr[0];

	return result;
}

int VirtualVNode::render(VFrame **video_out, 
				long video_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				long source_len, 
				int reverse,
				VRender *vrender,
				float step)
{
	if(real_module)
	{
		render_as_module(video_out, 
				video_out_position, 
				double_buffer,
				fragment_position,
				fragment_len, 
				real_position, 
				reverse,
				step,
				vrender);
	}
	else
	if(real_plugin || real_transition)
	{
		render_as_plugin(source_len,
				real_position,
				double_buffer,
				fragment_position,
				fragment_len,
				reverse);
	}
return 0;
}


int VirtualVNode::render_as_module(VFrame **video_out, 
				long video_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				int reverse,
				float step,
				VRender *vrender)
{
	this->reverse = reverse;
	VFrame **buffer_in = get_module_input(double_buffer, fragment_position);
	VFrame **buffer_out = get_module_output(double_buffer, fragment_position);

	render_fade(buffer_in, 
				buffer_out,
				fragment_len,
				real_position,
				real_module->fade,
				patch->automate,
				track->fade_autos,
				step);


// video is definitely in output buffer now
// overlay on the final output
	if(!real_module->mute)
	{
// Keep rendering unmuted fragments until finished.
		int auto_mute = patch->automate;
		int mute_constant;
		long mute_fragment;
		long mute_position = 0;

		while(fragment_len > 0)
		{
			mute_fragment = fragment_len;
// How many samples until the next mute?
			mute_constant = longest_mute_fragment(auto_mute,
										real_position,
										mute_fragment,
										(Autos*)((VTrack*)track)->mute_autos,
										&mute_before,
										&mute_after);

			if(mute_constant < 0)
			{
// Fragment is playable
				render_projector(buffer_out,
					&video_out[video_out_position],
					mute_fragment,
					real_position,
					patch->automate,
					step);
			}
			fragment_len -= mute_fragment;
			real_position += reverse ? -mute_fragment : mute_fragment;
			mute_position += mute_fragment;
		}
	}
return 0;
}

int VirtualVNode::render_fade(VFrame **input,         // start of input fragment
			VFrame **output,        // start of output fragment
			long buffer_len,      // fragment length in input scale
			long input_position,  // start of input fragment in project if forward / end of input fragment if reverse
			float constant,       // default opacity
			int automate,
			Autos *autos,
			float step)         // 1 for automation enabled
{
	init_automation(automate, 
					constant, 
					input_position,
					buffer_len * step,
					autos,
					&fade_before, 
					&fade_after);
//printf("VirtualVNode::render_fade %d %f\n", automate, constant);

// apply automation
	if(automate)
	{
		init_slope(autos, &fade_before, &fade_after);

// render remaining autos
		while(buffer_position < buffer_len)
		{
			get_slope(autos, buffer_len * step, buffer_position * step);

// copy only if buffers are different
			if(!(output[buffer_position]->equals(input[buffer_position])))
			{
				while(buffer_position < buffer_len && slope_position < slope_end)
				{
					value = slope * slope_position + slope_value;
					value += constant;

					if(value < 0) value = 0;
					if(value > 100) value = 100;
					output[buffer_position]->replace_from(input[buffer_position], (int)(value / 100 * VMAX), 
						mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint);
					buffer_position++;
					slope_position += step;
				}
			}
			else
			{
// apply fade directly
				while(buffer_position < buffer_len && slope_position < slope_end)
				{
					value = slope * slope_position + slope_value;
					value += constant;

					if(value < 0) value = 0;
					if(value > 100) value = 100;
					input[buffer_position]->apply_fade((int)(value / 100 * VMAX));

					buffer_position++;
					slope_position += step;
				}
			}

			advance_slope(autos);
		}
	}
	else
// ================================= no automation
	{
		value = constant;
		if(value < 0) value = 0;
		if(value > 100) value = 100;

// copy only if buffers are different
		if(!(output[buffer_position]->equals(input[buffer_position])))
		{
			while(buffer_position < buffer_len)
			{
				output[buffer_position]->replace_from(input[buffer_position], (int)(value / 100 * VMAX),
					mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint);
				buffer_position++;
			}
		}
		else
		if(value != 100)
		{
// apply to single buffer if fade is needed
			while(buffer_position < buffer_len)
			{
				output[buffer_position]->apply_fade((int)(value / 100 * VMAX));
				buffer_position++;
			}
		}
	}
return 0;
}

int VirtualVNode::render_projector(VFrame **input,
			VFrame **output,
			long fragment_len,
			long real_position,  // Start of input fragment in project if forward.  End of input fragment if reverse.
			int automate,
			float step)
{
	float in_x1, in_y1, in_x2, in_y2;
	float out_x1, out_y1, out_x2, out_y2;
	float float_real_position = real_position;

	if(reverse) step *= -1;
	for(buffer_position = 0; 
		buffer_position < fragment_len; 
		buffer_position++,
		float_real_position += step)
	{
		((VTrack*)track)->get_projection(in_x1, in_y1, in_x2, in_y2,
							out_x1, out_y1, out_x2, out_y2,
							frame_w, frame_h, (long)float_real_position,
							&projector_before, &projector_after);

		if(out_x2 > out_x1 && out_y2 > out_y1 && in_x2 > in_x1 && in_y2 > in_y1)
		{
// overlay it depending on the function for this module
			transfer_from(output[buffer_position], input[buffer_position], 
				in_x1, in_y1, in_x2, in_y2,
				out_x1, out_y1, out_x2, out_y2, VMAX,
				mwindow->preferences->video_use_alpha, mwindow->preferences->video_floatingpoint, 
				mwindow->preferences->video_interpolate, ((VModule*)real_module)->mode);
		}
	}
return 0;
}

int VirtualVNode::transfer_from(VFrame *frame_out, VFrame *frame_in, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate, 
		int mode)
{
	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, interpolate, mode))
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, interpolate, mode);
	}
	overlayer->overlay(frame_out, frame_in,
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha);
return 0;
}
