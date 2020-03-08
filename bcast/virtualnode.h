#ifndef VIRTUALNODE_H
#define VIRTUALNODE_H

#include "arraylist.h"
#include "auto.inc"
#include "autos.inc"
#include "floatauto.inc"
#include "floatautos.inc"
#include "mainwindow.inc"
#include "maxbuffers.h"
#include "patch.h"
#include "plugin.inc"
#include "pluginbuffer.inc"
#include "pluginserver.inc"
#include "track.inc"
#include "transition.inc"

// The virtual node makes up the virtual console.
// It can be either a virtual module or a virtual plugin.


class VirtualNode
{
public:
	VirtualNode(MainWindow *mwindow, 
		Module *real_module, 
		Plugin *real_plugin,
		Transition *real_transtion, 
		Patch *patch, 
		Track *track, 
		VirtualNode *parent_module, 
		PluginBuffer **input,
		PluginBuffer **output,
		long buffer_size,
		long fragment_size, 
		int input_is_master,
		int output_is_master,
		int double_buffers, 
		int in,
		int out);

	virtual ~VirtualNode();

// derived node creates PluginBuffers here
	virtual int create_output_buffer() { return 0; };
	virtual int create_input_buffer() { return 0; };
	virtual int delete_input_buffer() { return 0; };
	virtual int delete_output_buffer() { return 0; };
	virtual int delete_buffer_ptrs() { return 0; };


// expand into plugins
	int expand(int duplicate, long current_position);
// create buffers and convenience pointers
	int expand_buffers();
// create convenience pointers to shared memory depending on the data type
	virtual int create_buffer_ptrs() { return 0; };
// create a node for a module
	int attach_virtual_module(int plugin_number, int duplicate, long current_position, Transition *real_transition);
// create a node for a plugin
	int attach_virtual_plugin(int plugin_number, int duplicate, long current_position, Transition *real_transition);
	virtual VirtualNode* create_module(Plugin *real_plugin, 
							Module *real_module, 
							Transition *real_transition, 
							VirtualNode *parent_module, 
							Patch *patch, 
							Track *track) { return 0; };
	virtual VirtualNode* create_plugin(Plugin *real_plugin, Transition *real_transition) { return 0; };

	int render_as_plugin(long source_len,
						long source_position,
						int double_buffer,
						long fragment_position,
						long fragment_len,
						int reverse);

	int get_plugin_input(int &double_buffer_in, long &fragment_position_in,
					int &double_buffer_out, long &fragment_position_out,
					int double_buffer, long fragment_position);

// Get the order to render modules and plugins attached to this.
// Return 1 if not completed after this pass.
	int sort(ArrayList<VirtualNode*>*render_list);

// pointers to double buffers of shared memory to be used as buffers
// Top level modules always have double buffers from the render engine and 
// have output pointing to input.
// Some plugins access their own single buffers because their in/out settings
// are off.
// (PluginBuffer*)[which double buffer]
	PluginBuffer *output[MAX_BUFFERS];
	PluginBuffer *input[MAX_BUFFERS];

// module which created this node.
	VirtualNode *parent_module;
// Virtual transitions this module owns
	VirtualNode *virtual_transition;
// virtual plugins this module owns
	VirtualNode *virtual_plugins[PLUGINS];
// use these to determine if this node is a plugin or module
	Module *real_module;  // when this node is a module
// when this node is a plugin
	Plugin *real_plugin;
// When this node is a transition
	Transition *real_transition;

	Patch *patch;
	Track *track;
	MainWindow *mwindow;

// for rendering need to know if the buffer is a master or copy
// These are set in expand()
	int input_is_master;
	int output_is_master;
	int double_buffers;       // number of buffers for master buffers
	long buffer_size;         // number of units in a master segment
	long fragment_size;       // number of units in a node segment
	int plugin_type;          // type of plugin in case user changes it
	int render_count;         // times this plugin has been added to the render list
	int waiting_real_plugin;  //  real plugin tests this to see if virtual plugin is waiting on it when sorting
// for deleting need to know if buffers are owned by someone else
	int shared_input;
	int shared_output;
// where this node should get its input and output from
	int in;
	int out;
// module needs to know where the input data for the next process is
	int data_in_input;
// plugin needs to know what buffer number each fragment position corresponds to
	int plugin_buffer_number;

// This is set up and deleted by the real plugin.
	PluginServer *plugin_server;

// Mute automation.
// Return whether the next samples are muted and store the longest
// fragment until the next mute auto in buffer_len.
	int longest_mute_fragment(int &automate, 
				long input_position,
				long &buffer_len,
				Autos *autos,
				Auto **before, 
				Auto **after);

// convenience routines for fade automation
	int init_automation(int &automate, 
				float &constant, 
				long input_position,
				long buffer_len,
				Autos *autos,
				Auto **before, 
				Auto **after);

	int init_slope(Autos *autos, Auto **before, Auto **after);
	int get_slope(Autos *autos, long buffer_len, long buffer_position);
	int advance_slope(Autos *autos);

protected:
// ======================= mute automation
// Persistant variables for mute
	Auto *mute_before, *mute_after;
// Temporary variables for mute
	Auto *current_mute;
	

// ======================= fade automation
// persistant variables for fade
	Auto *fade_before, *fade_after;

// ======================= plugin automation
	FloatAutos *plugin_autos;
	FloatAuto *plugin_auto_before, *plugin_auto_after;

// temporary variables for automation
	Auto *current_auto;
	float slope_value;
	float slope_start;
	float slope_end;
	float slope_position;
	float slope;
	float value;

	int reverse;  // Temporary set for each render
	long input_start, input_end;
	long buffer_position; // position in both input and output buffer

private:
	int sort_as_module(ArrayList<VirtualNode*>*render_list, int &result, int &total_result);
	int sort_as_plugin(ArrayList<VirtualNode*>*render_list, int &result, int &total_result);
	int expand_as_module(int duplicate, long current_position);
	int expand_as_plugin(int duplicate);
	int expand_as_transition(int duplicate);
};



#endif
