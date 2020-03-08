#include <string.h>
#include "console.h"
#include "datatype.h"
#include "mainwindow.h"
#include "modules.h"
#include "playabletracks.h"
#include "pluginbuffer.h"
#include "renderengine.h"
#include "tracks.h"
#include "vframe.h"
#include "videodevice.h"
#include "virtualvnode.h"
#include "vmodule.h"
#include "vrender.h"
#include "vrenderthread.h"

VRenderThread::VRenderThread(MainWindow *mwindow, VRender *vrender)
 : CommonRenderThread(mwindow, vrender)
{
	this->vrender = vrender;
}

VRenderThread::~VRenderThread()
{
}

int VRenderThread::init_rendering(int duplicate)
{
	playable_tracks = new PlayableTracks(mwindow, 
							vrender->current_position, 
							render_engine->reverse, 
							TRACK_VIDEO);

	total_tracks = playable_tracks->total;

	allocate_input_buffers();
	build_virtual_console(duplicate, vrender->current_position);
	sort_virtual_console();
	mwindow->console->modules->vrender_init(duplicate);
	mwindow->tracks->vrender_init(duplicate, vrender->current_position);
return 0;
}

int VRenderThread::build_virtual_console(int duplicate, long current_position)
{
// allocate the virtual modules and fork plugins
	total_virtual_modules = total_tracks;
	virtual_modules = new VirtualNode*[total_virtual_modules];
	PluginBuffer *buffer[MAX_BUFFERS];
	int i, j;

// build virtual console table
	for(i = 0, j; i < playable_tracks->total; i++)
	{
// create a double buffer of just one track
		for(j = 0; j < total_buffers; j++)
			buffer[j] = buffer_in[j][i];

		virtual_modules[i] = new VirtualVNode(mwindow, 
			playable_tracks->values[i]->get_module_of(), 
			0, 
			0, 
			playable_tracks->values[i]->get_patch_of(), 
			playable_tracks->values[i], 
			0,
			buffer,
			buffer,
			vrender->input_length,
			vrender->vmodule_render_fragment, 
			total_buffers,
			1,
			1,
			1,
			1,
			vrender->track_w, 
			vrender->track_h);

		virtual_modules[i]->expand(duplicate, current_position);
	}
return 0;
}

PluginBuffer** VRenderThread::allocate_input_buffer(int double_buffer)
{
	PluginBuffer **result = new PluginBuffer*[total_tracks];
	int i;
// get pointers to frames in the input buffers
	buffer_in_ptr = new VFrame**[total_tracks];
	shared_buffer_in_ptr = new PluginBuffer**[total_tracks];

	for(i = 0; i < total_tracks; i++)
	{
		result[i] = new PluginBuffer(vrender->input_length * vrender->track_w * vrender->track_h + 1, sizeof(VPixel));

		buffer_in_ptr[i] = new VFrame*[vrender->input_length];
		shared_buffer_in_ptr[i] = new PluginBuffer*[vrender->input_length];

		VPixel *raw = (VPixel*)result[i]->get_data();
		for(int frame = 0; frame < vrender->input_length; frame++)
		{
			buffer_in_ptr[i][frame]
				 = new VFrame((unsigned char*)(&raw[frame * vrender->track_w * vrender->track_h]), 
				 	vrender->track_w, 
					vrender->track_h);
			shared_buffer_in_ptr[i][frame] = result[i];
		}
	}
	
	return result;
}

int VRenderThread::delete_input_buffer(int buffer)
{
	for(int which_track = 0; which_track < total_tracks; which_track++)
	{
		for(int frame = 0; frame < vrender->input_length; frame++)
		{
			delete buffer_in_ptr[which_track][frame];
		}
		delete shared_buffer_in_ptr[which_track];
		delete buffer_in_ptr[which_track];
		delete buffer_in[0][which_track];
	}
	delete shared_buffer_in_ptr;
	delete buffer_in_ptr;
	delete buffer_in[0];
return 0;
}

int VRenderThread::stop_rendering(int duplicate)
{
// stop plugins
	mwindow->console->modules->vrender_stop(duplicate);
	mwindow->tracks->vrender_stop(duplicate);

	delete_virtual_console();
	delete_input_buffers();
return 0;
}



int VRenderThread::process_buffer(int buffer,
							  long input_len,      // same for double and normal speed
							  long input_position, // start of buffer in project if forward / end of buffer if reverse
							  long absolute_position) // not used in video
{
	int i, j, k;
// length and lowest numbered frame of fragment in input buffer
	long fragment_len, fragment_position;
// generic buffer
	VFrame **current_buffer;
// starting frame of fragment in project
	long real_position;

// process entire input buffer one output buffer at a time
	for(fragment_position = 0; fragment_position < input_len; )
	{
// test for end of input buffer
		fragment_len = vrender->vmodule_render_fragment;
		if(fragment_position + fragment_len > input_len)
			fragment_len = input_len - fragment_position;

// test for end of output buffer
		if(vrender->video_out_position + fragment_len > vrender->playback_buffer)
			fragment_len = vrender->playback_buffer - vrender->video_out_position;

// clear output buffer
		current_buffer = &vrender->video_out[vrender->video_out_position];
		for(j = 0;  j < fragment_len; j++)
			current_buffer[j]->clear_frame();

// get the start of the fragment in the project
		real_position = render_engine->reverse ? input_position - fragment_position : input_position + fragment_position;

// render nodes in sorted list
		for(i = 0; i < render_list.total; i++)
		{
			((VirtualVNode*)render_list.values[i])->render(vrender->video_out, 
					vrender->video_out_position, 
					buffer,
					fragment_position,
					fragment_len, 
					real_position, 
					vrender->source_length, 
					render_engine->reverse,
					vrender,
					1);
		}

// advance counters
		fragment_position += fragment_len;
		vrender->video_out_position += fragment_len;

// rotate the output position
		if(vrender->video_out_position >= vrender->playback_buffer)
		{
			vrender->video_out_position = 0;
		}
	}
//printf("VRenderThread::process_buffer 2\n");
return 0;
}

int VRenderThread::send_last_output_buffer()
{
return 0;
}
