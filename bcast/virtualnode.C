#include <string.h>
#include "auto.h"
#include "autos.h"
#include "console.h"
#include "edit.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "patchbay.h"
#include "plugin.h"
#include "pluginbuffer.h"
#include "pluginserver.h"
#include "tracks.h"
#include "transition.h"
#include "virtualnode.h"

VirtualNode::VirtualNode(MainWindow *mwindow, 
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
		int out)
{
	this->mwindow = mwindow;
	this->real_module = real_module;
	this->real_plugin = real_plugin;
	this->real_transition = real_transition;
	this->patch = patch;
	this->track = track;
	this->parent_module = parent_module;
	this->buffer_size = buffer_size;
	this->fragment_size = fragment_size;
	this->double_buffers = double_buffers;

// assume parent is supplying buffers for now
	for(int i = 0; i < double_buffers; i++)
	{
		this->output[i] = output[i];
		this->input[i] = input[i];
	}

	shared_input = 1;
	this->input_is_master = input_is_master;
	shared_output = 1;
	this->output_is_master = output_is_master;
	this->in = in;
	this->out = out;
	virtual_transition = 0;
	for(int i = 0; i < PLUGINS; i++)
	{
		virtual_plugins[i] = 0;
	}

	render_count = 0;
	plugin_server = 0;
	plugin_type = 0;
	waiting_real_plugin = 0;
	plugin_buffer_number = 0;
	fade_before = fade_after = 0;
	mute_before = mute_after = 0;
	plugin_autos = 0;
	plugin_auto_before = plugin_auto_after = 0;
}

VirtualNode::~VirtualNode()
{
	if(virtual_transition) delete virtual_transition;
	for(int i = 0; i < PLUGINS; i++)
	{
		if(virtual_plugins[i]) delete virtual_plugins[i];
	}
}

int VirtualNode::expand(int duplicate, long current_position)
{
	expand_buffers();

// module needs to know where the input data for the next process is
	data_in_input = 1;
	if(real_module)
	{
		expand_as_module(duplicate, current_position);
	}
	else
	if(real_plugin)
	{
// attach to a real plugin for a plugin
// plugin always takes data from input to output
		expand_as_plugin(duplicate);
	}
	else
	if(real_transition)
	{
		expand_as_transition(duplicate);
	}
return 0;
}

int VirtualNode::expand_buffers()
{
// get new buffers where needed
	if(!out)
	{
// create shared memory for the output since parent doesn't want it
		create_output_buffer();
		shared_output = 0;
		output_is_master = 0;
	}

	if(!in)
	{
// create shared memory for null input since parent isn't giving it
		create_input_buffer();
		shared_input = 0;
		input_is_master = 0;
	}

// give the derived node its data types
	create_buffer_ptrs();
return 0;
}

int VirtualNode::expand_as_module(int duplicate, long current_position)
{
	Transition *transition = 0;

// Expand the transition if this is a track has a transition
	if(track && (transition = track->get_transition(current_position)))
	{
		if(transition->on)
		{
			int plugin_type = transition->plugin_type;
			if(plugin_type == 3)
			{
// transition is a module
				attach_virtual_module(0, duplicate, current_position, transition);
			}
			else
			{
// transition is a plugin
				attach_virtual_plugin(0, duplicate, current_position, transition);
			}
		}
	}

// expand the plugins for a module
	for(int i = 0; i < PLUGINS; i++)
	{
		virtual_plugins[i] = 0;
		if(real_module->plugins[i]->on)
		{
			int plugin_type = real_module->plugins[i]->plugin_type;
			if(plugin_type == 3)
			{
// plugin is a module
				attach_virtual_module(i, duplicate, current_position, 0);
			}
			else
			if(plugin_type)
			{
// plugin is a plugin
				attach_virtual_plugin(i, duplicate, current_position, 0);
			}
		}
	}
return 0;
}

int VirtualNode::expand_as_plugin(int duplicate)
{
	plugin_type = real_plugin->plugin_type;
	if(plugin_type == 2)
	{
// Attached to a shared plugin.
// Get the real plugin it's attached to.
// Redirect the real_plugin to the shared plugin.
		int real_module_number = real_plugin->shared_plugin_location.module;
		int real_plugin_number = real_plugin->shared_plugin_location.plugin;
		Module *real_module = mwindow->console->modules->module_number(real_module_number);
		real_plugin = real_module->plugins[real_plugin_number];

// Real plugin not on then null it.
		if(!real_plugin->on) real_plugin = 0;
	}

// Add to real plugin's list of virtual plugins for configuration updates
// and plugin_server initializations.
// Input and output are taken care of when the parent module creates this plugin.
// Get number for passing to render.
	if(real_plugin)
	{
		plugin_buffer_number = real_plugin->attach_virtual_plugin(this);
// Get automation for the plugin.
// Need to test whether the automation patch is on.
		if(real_plugin->module->get_patch_of()->automate)
			plugin_autos = real_plugin->module->get_track_of()->plugin_autos[real_plugin->get_plugin_number() - 1];
		else
			plugin_autos = 0;
	}
return 0;
}

int VirtualNode::expand_as_transition(int duplicate)
{
	plugin_type = real_transition->plugin_type;

	if(plugin_type == 2)
	{
// Aattached to a shared plugin.
// Get the real plugin it's attached to.
// Redirect the real_plugin to the shared plugin.
		int real_module_number = real_transition->shared_plugin_location.module;
		int real_plugin_number = real_transition->shared_plugin_location.plugin;
		Module *real_module = mwindow->console->modules->module_number(real_module_number);
		real_plugin = real_module->plugins[real_plugin_number];

// Real plugin not on then null it.
		if(!real_plugin->on) real_plugin = 0;

		if(real_plugin)
			plugin_buffer_number = real_plugin->attach_virtual_plugin(this);
	}
	else
	{
		if(!real_transition->on) real_transition = 0;

// Add to the real plugin's list of virtual plugins for configuration updates
// and plugin_server initializations.
// Input and output are taken care of when the parent module creates this plugin.
// Get number for passing to render.
		if(real_transition)
			plugin_buffer_number = real_transition->attach_virtual_plugin(this);
	}
return 0;
}

int VirtualNode::attach_virtual_module(int plugin_number, int duplicate, long current_position, Transition *real_transition)
{
	if(!real_transition)
	{
		int real_module_number = real_module->plugins[plugin_number]->shared_module_location.module;
		Module *real_module = mwindow->console->modules->module_number(real_module_number);
		Plugin *real_plugin = this->real_module->plugins[plugin_number];
		Patch *patch = mwindow->patches->number(real_module_number);
		Track *track = mwindow->tracks->number(real_module_number);

		virtual_plugins[plugin_number]
			 = create_module(real_plugin, real_module, 0, parent_module, patch, track);
		virtual_plugins[plugin_number]->expand(duplicate, current_position);

// working data is now in output
		if(real_plugin->out) data_in_input = 0;
	}
	else
	{
		int real_module_number = real_transition->shared_module_location.module;
		Module *real_module = mwindow->console->modules->module_number(real_module_number);
		Patch *patch = mwindow->patches->number(real_module_number);
		Track *track = mwindow->tracks->number(real_module_number);

		virtual_transition
			= create_module(0, real_module, real_transition, parent_module, patch, track);
		virtual_transition->expand(duplicate, current_position);

// working data is now in output
		if(real_transition->out) data_in_input = 0;
	}
return 0;
}

int VirtualNode::attach_virtual_plugin(int plugin_number, int duplicate, long current_position, Transition *real_transition)
{
	if(!real_transition)
	{
		Plugin *real_plugin = real_module->plugins[plugin_number];

		if(real_plugin->on)
		{
			virtual_plugins[plugin_number] = create_plugin(real_plugin, 0);
			virtual_plugins[plugin_number]->expand(duplicate, current_position);

// working data is now in output
			if(real_plugin->out) data_in_input = 0;
		}
	}
	else
	{
		if(real_transition->on)
		{
			virtual_transition = create_plugin(0, real_transition);
			virtual_transition->expand(duplicate, current_position);

// working data is now in output
			if(real_transition->out) data_in_input = 0;
		}
	}
return 0;
}

int VirtualNode::sort(ArrayList<VirtualNode*>*render_list)
{
	int result = 0, total_result = 0;

	if(real_module)
	{
		sort_as_module(render_list, result, total_result);
	}
	else
	if(real_plugin || real_transition)
	{
		sort_as_plugin(render_list, result, total_result);
	}

	if(!result && total_result) result = total_result;
// if a plugin that wasn't patched out couldn't be rendered, try again

	return result;
return 0;
}

int VirtualNode::sort_as_module(ArrayList<VirtualNode*>*render_list, int &result, int &total_result)
{
// stop when rendering can't continue without another higher level module
	if(virtual_transition)
	{
		result = virtual_transition->sort(render_list);
		
		if(result && !virtual_transition->out)
		{
			total_result = 1;
			result = 0;
// couldn't render the last plugin but it wasn't patched out so continue to next plugin
		}
	}

	for(int i = 0; i < PLUGINS && !result; i++)
	{
// stop when rendering can't continue without another higher level module
		if(virtual_plugins[i])
		{
			result = virtual_plugins[i]->sort(render_list);

			if(result && !virtual_plugins[i]->out)
			{
				total_result = 1;
				result = 0;
// couldn't render the last plugin but it wasn't patched out so continue to next plugin
			}
		}
	}

// all plugins rendered or not patched out
// render this module
	if(render_count == 0 && !result)
	{
		render_list->append(this);
		render_count++;
		result = 0;
	}
return 0;
}

int VirtualNode::sort_as_plugin(ArrayList<VirtualNode*>*render_list, int &result, int &total_result)
{
// Plugin server does not exist at this point.
// need to know if plugin requires all inputs to be armed before rendering
	int multichannel = 0, singlechannel = 0;

	if(plugin_type == 1 || plugin_type == 2)
	{
		multichannel = real_plugin ? real_plugin->multichannel_shared(1) :
									real_transition->multichannel_shared(1);
		
		singlechannel = real_plugin ? real_plugin->singlechannel() :
									real_transition->singlechannel();
	}

	if(plugin_type == 1 && !multichannel)
	{
// unshared single channel plugin
// render now
		if(!render_count)
		{
			render_list->append(this);
			render_count++;
			result = 0;
		}
	}
	else
	if(plugin_type == 2 || multichannel)
	{
// Shared plugin
		if(!render_count)
		{
			if(singlechannel)
			{
// shared single channel plugin
// render now
				render_list->append(this);
				render_count++;
				result = 0;
			}
			else
			{
// shared multichannel plugin
// all buffers must be armed before rendering at the same time
				if(!waiting_real_plugin)
				{
					waiting_real_plugin = 1;
					if(real_plugin)
						result = real_plugin->sort(this);
					else
					if(real_transition)
						result = real_transition->sort(this);

					render_list->append(this);
					render_count++;
				}
				else
				{
// Assume it was rendered later in the first pass
					result = 0;
				}
			}
		}
	}
return 0;
}

int VirtualNode::get_plugin_input(int &double_buffer_in, long &fragment_position_in,
							int &double_buffer_out, long &fragment_position_out,
							int double_buffer, long fragment_position)
{
	if(input_is_master)
	{
		double_buffer_in = double_buffer;
		fragment_position_in = fragment_position;
	}
	else
	{
		double_buffer_in = 0;
		fragment_position_in = 0;
	}

	if(output_is_master)
	{
		double_buffer_out = double_buffer;
		fragment_position_out = fragment_position;
	}
	else
	{
		double_buffer_out = 0;
		fragment_position_out = 0;
	}
return 0;
}

int VirtualNode::render_as_plugin(long source_len,
		long source_position,
		int double_buffer,
		long fragment_position,
		long fragment_len,
		int reverse)
{
// need numbers for actual buffers
	int double_buffer_in, double_buffer_out;
	long fragment_position_in, fragment_position_out;
	int multichannel = 0;
	AttachmentPoint *attachmentpoint;

// Redirect the attachmentpoint to the shared plugin or standalone plugin.
	if(real_transition)
	{
		if(plugin_type == 2)
			attachmentpoint = real_plugin;
		else
			attachmentpoint = real_transition;

// Override the source positions if a transition.
		source_len = real_transition->edit->length;

		if(reverse)
			source_position = real_transition->edit->startproject + real_transition->edit->length - source_position;
		else
			source_position -= real_transition->edit->startproject;
	}
	else
	{
		attachmentpoint = real_plugin;
	}

// Abort if no plugin
	if(!attachmentpoint) return 0;
	if(!attachmentpoint->on) return 0;

	get_plugin_input(double_buffer_in, fragment_position_in,
					double_buffer_out, fragment_position_out,
					double_buffer, fragment_position);

	if(plugin_type == 1 || plugin_type == 2) 
	{
		if(!attachmentpoint->plugin_server)
			return 0;
		else
			multichannel = attachmentpoint->multichannel_shared(0);
	}

	if(plugin_type == 1 && !multichannel)
	{
// standalone plugin
// plugins copy from their input to their output
		plugin_server->arm_buffer(plugin_buffer_number,
							fragment_position_in,
							fragment_position_out,
							double_buffer_in,
							double_buffer_out);
		plugin_server->set_automation(plugin_autos, &plugin_auto_before, &plugin_auto_after, reverse);
		plugin_server->process_realtime(source_len, source_position, fragment_len);
	}
	else
	if(plugin_type == 2 || multichannel)
	{
// render a shared plugin
// shared single channel plugin can be processed now
		if(!plugin_server->multichannel)
		{
			plugin_server->arm_buffer(plugin_buffer_number,
							fragment_position_in,
							fragment_position_out,
							double_buffer_in,
							double_buffer_out);
			plugin_server->set_automation(plugin_autos, &plugin_auto_before, &plugin_auto_after, reverse);
			plugin_server->process_realtime(source_len, 
											source_position, 
											fragment_len);
		}
		else
		{
// try multi channel plugin now
// flag real plugin by setting render count here
			render_count = 0;
			attachmentpoint->render(double_buffer_in, double_buffer_out, 
									fragment_position_in, fragment_position_out,
									fragment_len, plugin_buffer_number, 
									source_position, source_len,
									plugin_autos, 
									&plugin_auto_before, 
									&plugin_auto_after);
		}
	}
return 0;
}



int VirtualNode::longest_mute_fragment(int &automate, 
			long input_position,
			long &buffer_len,
			Autos *autos,
			Auto **before, 
			Auto **after)
{
	float mute_constant = 0;
	long mute_len;

	init_automation(automate, 
				mute_constant, 
				input_position,
				buffer_len,
				autos,
				before, 
				after);

// Muted all the way through
	if(!automate && mute_constant > 0) return 1;
// Playable all the way through
	if(!automate && mute_constant < 0) return -1;

// Changes somewhere
	current_auto = *before;
	if(!current_auto)
	{
		current_auto = autos->first;
	}

	if(current_auto)
	{
		mute_constant = current_auto->value;
		current_auto = current_auto->next;
	}
	else
	{
		mute_constant = -1;
		automate = 0;
	}

	if(current_auto) 
	{
		mute_len = reverse ? current_auto->position - input_start : 
			current_auto->position - input_start;
	}
	else
	{
		mute_len = buffer_len;
	}

	if(buffer_len > mute_len) buffer_len = mute_len;
	return (int)mute_constant;
return 0;
}




int VirtualNode::init_automation(int &automate, 
				float &constant, 
				long input_position,
				long buffer_len,
				Autos *autos,
				Auto **before, 
				Auto **after)
{
	return autos->init_automation(buffer_position,
				input_start, 
				input_end, 
				automate, 
				constant, 
				input_position,
				buffer_len,
				before, 
				after,
				reverse);
return 0;
}

int VirtualNode::init_slope(Autos *autos, Auto **before, Auto **after)
{
	return autos->init_slope(&current_auto,
				slope_start, 
				slope_value,
				slope_position, 
				input_start, 
				input_end, 
				before, 
				after,
				reverse);
return 0;
}

int VirtualNode::get_slope(Autos *autos, long buffer_len, long buffer_position)
{
	return autos->get_slope(&current_auto, 
				slope_start, 
				slope_end, 
				slope_value, 
				slope, 
				buffer_len, 
				buffer_position,
				reverse);
return 0;
}

int VirtualNode::advance_slope(Autos *autos)
{
	return autos->advance_slope(&current_auto, 
				slope_start, 
				slope_value,
				slope_position, 
				reverse);
return 0;
}
