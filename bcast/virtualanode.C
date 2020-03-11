#include <string.h>
#include "amodule.h"
#include "arender.h"
#include "atrack.h"
#include "console.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "module.h"
#include "patchbay.h"
#include "plugin.h"
#include "pluginbuffer.h"
#include "track.h"
#include "transition.h"
#include "virtualanode.h"

VirtualANode::VirtualANode(MainWindow *mwindow, 
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
		int out_channels)
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
	int i;
	for(i = 0; i < double_buffers; i++)
	{
		input_ptr[i] = 0;
		output_ptr[i] = 0;
	}
	this->out_channels = out_channels;
	for(i = 0; i < out_channels; i++)
	{
		pan_before[i] = pan_after[i] = 0;
	}
}

VirtualANode::~VirtualANode()
{
	if(!shared_output) delete_output_buffer();
	if(!shared_input) delete_input_buffer();
	delete_buffer_ptrs();
}

int VirtualANode::create_output_buffer()
{
	output[0] = new PluginBuffer(fragment_size, sizeof(float));
return 0;
}

int VirtualANode::create_input_buffer()
{
	input[0] = new PluginBuffer(fragment_size, sizeof(float));
// zero the input buffer
	float *input_ptr = (float*)input[0]->get_data();
	for(int i = 0; i < fragment_size; i++)
	{
		input_ptr[i] = 0;
	}
return 0;
}

int VirtualANode::delete_output_buffer()
{
	delete output[0];
return 0;
}

int VirtualANode::delete_input_buffer()
{
	delete input[0];
return 0;
}

int VirtualANode::create_buffer_ptrs()
{
	int i;
	for(i = 0; i < (input_is_master ? double_buffers : 1); i++)
	{
		input_ptr[i] = (float*)input[i]->get_data();
	}
	for(i = 0; i < (output_is_master ? double_buffers : 1); i++)
	{
		output_ptr[i] = (float*)output[i]->get_data();
	}
return 0;
}

int VirtualANode::delete_buffer_ptrs()
{
return 0;
}

VirtualNode* VirtualANode::create_module(Plugin *real_plugin, 
					Module *real_module, 
					Transition *real_transition, 
					VirtualNode *parent_module, 
					Patch *patch, 
					Track *track)
{
	return new VirtualANode(mwindow, 
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
		data_in_input ? input_is_master : output_is_master,
		output_is_master,
		real_transition ? real_transition->in : real_plugin->in,
		real_transition ? real_transition->out : real_plugin->out,
		out_channels);
}

VirtualNode* VirtualANode::create_plugin(Plugin *real_plugin, Transition *real_transition)
{
	return new VirtualANode(mwindow, 
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
		out_channels);
}

float* VirtualANode::get_module_input(int double_buffer, long fragment_position)
{
	float *result;
	if(data_in_input)
	{
		if(input_is_master) 
			result = input_ptr[double_buffer] + fragment_position;
		else
			result = input_ptr[0];
	}
	else
	{
		if(output_is_master)
			result = output_ptr[double_buffer] + fragment_position;
		else
			result = output_ptr[0];
	}
	return result;
}

float* VirtualANode::get_module_output(int double_buffer, long fragment_position)
{
	float *result;

	if(output_is_master)
		result = output_ptr[double_buffer] + fragment_position;
	else
		result = output_ptr[0];

	return result;
}

int VirtualANode::render(float **audio_out, 
				long audio_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				long source_length, 
				int reverse,
				ARender *arender)
{
	if(real_module)
	{
		render_as_module(audio_out, 
				audio_out_position, 
				double_buffer,
				fragment_position,
				fragment_len, 
				real_position, 
				reverse,
				arender);
	}
	else
	if(real_plugin || real_transition)
	{
		render_as_plugin(source_length, 
				real_position,
				double_buffer,
				fragment_position,
				fragment_len,
				reverse);
	}
return 0;
}

int VirtualANode::render_as_module(float **audio_out, 
				long audio_out_position, 
				int double_buffer,
				long fragment_position,
				long fragment_len, 
				long real_position, 
				int reverse,
				ARender *arender)
{
	int in_output = 0;
	this->reverse = reverse;
	float *buffer_in = get_module_input(double_buffer, fragment_position);
	float *buffer_out = get_module_output(double_buffer, fragment_position);

	if(((AModule*)real_module)->inv)
	{
		for(int i = 0; i < fragment_len; i++)
		{
			buffer_out[i] = -buffer_in[i];
		}
		in_output = 1;
	}

	render_float_buffer(in_output ? buffer_out : buffer_in, 
				buffer_out,
				fragment_len,
				real_position,
				real_module->fade,
				INFINITYGAIN,
				patch->automate,
				track->fade_autos,
				&fade_before,
				&fade_after,
				0,
				1);

// audio is definitely in output buffer now
// get the peak if the console isn't hidden and there is a meter
	if(mwindow->gui && !mwindow->console->gui->get_hidden() && ((AModule*)real_module)->peak_history)
	{
		float max = 0, min = 0, peak;
		long meter_render_end;   // next sample to update peak on
		long current_peak = arender->current_peak;

		for(int i = 0; i < fragment_len; )
		{
			meter_render_end = i + arender->meter_render_fragment;
			if(meter_render_end > fragment_len) meter_render_end = fragment_len;
			max = 0;
			min = 0;
			
			for( ; i < meter_render_end; i++)
			{
				if(buffer_out[i] > max) max = buffer_out[i];
				else
				if(buffer_out[i] < min) min = buffer_out[i];
			}

			if(fabs(max) > fabs(min)) 
				peak = fabs(max);
			else
				peak = fabs(min);

			((AModule*)real_module)->update_peak(current_peak, peak);
			current_peak = arender->get_next_peak(current_peak);
		}
	}

// process pans and copy the output to the output channels
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
										(Autos*)((ATrack*)track)->mute_autos,
										&mute_before,
										&mute_after);

			if(mute_constant < 0)
			{
// Fragment is playable
				for(int i = 0; i < out_channels; i++)
				{
					render_float_buffer(buffer_out + mute_position, 
								&audio_out[i][audio_out_position + mute_position],
								mute_fragment,
								real_position,
								((AModule*)real_module)->pan[i],
								INFINITYGAIN,
								patch->automate,
								((ATrack*)track)->pan_autos[i],
								&pan_before[i],
								&pan_after[i],
								1,
								0);
				}
			}
			fragment_len -= mute_fragment;
			real_position += reverse ? -mute_fragment : mute_fragment;
			mute_position += mute_fragment;
		}
	}
return 0;
}

int VirtualANode::render_float_buffer(float *input,         // start of input fragment
									float *output,        // start of output fragment
									long buffer_len,      // fragment length in input scale
									long input_position,  // starting sample of input buffer in project
									float constant,       // default scalar in DB
									float minvalue,       // minimum value for automating constant
									int automate,         // 1 for automation enabled
									Autos *autos,	 // link list of autos
									Auto **before,
									Auto **after,
									int additive,         // for pan output
									int use_db)
{
	float db_value;

	init_automation(automate, 
					constant, 
					input_position,
					buffer_len,
					autos,
					before, 
					after);

// apply automation
	if(automate)
	{
		init_slope(autos, before, after);

// render remaining autos
		while(buffer_position < buffer_len)
		{
			get_slope(autos, buffer_len, buffer_position);

			if(!additive)
			{
// straight copy
				while(buffer_position < buffer_len && slope_position < slope_end)
				{
					value = slope * slope_position + slope_value;
					value += constant;
					if(use_db) 
						value = db.fromdb(value);
					value *= input[buffer_position];

					output[buffer_position] = value;      // *

					buffer_position++;
					slope_position++;
				}
			}
			else
			{
// additive
				while(buffer_position < buffer_len && slope_position < slope_end)
				{
					value = slope * slope_position + slope_value;
					value += constant;
					if(use_db) value = db.fromdb(value);
					else
					if(value < 0) value = 0;
					value *= input[buffer_position];

					output[buffer_position] += value;      // *

					buffer_position++;
					slope_position++;
				}
			}

			advance_slope(autos);
		}
	}
	else
// ================================= no automation
	{
		if(use_db) 
			value = db.fromdb(constant);
		else 
			value = constant;

		if(value < 0) value = 0;
// straight copy
// overwrite output buffers
		if(!additive)
		{
// only if buffers are different or the fade is needed
			if(output != input || value != 1)
			{
				while(buffer_position < buffer_len)
				{
					output[buffer_position] = value * input[buffer_position];
					buffer_position++;
				}
			}
		}
		else
		{
// combine output buffers
			float this_value;

			while(buffer_position < buffer_len)
			{
				this_value = value * input[buffer_position];
				output[buffer_position] += this_value;
				buffer_position++;
			}
		}
	}
return 0;
}
