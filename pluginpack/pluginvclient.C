#include <string.h>
#include "pluginvclient.h"


PluginVClient::PluginVClient(int argc, char *argv[])
 : PluginClient(argc, argv)
{
	video_in = 0;
	video_out = 0;
}

PluginVClient::~PluginVClient()
{
}

int PluginVClient::plugin_is_video()
{
	return 1;
}

// Run before starting the realtime plugin
int PluginVClient::create_buffer_ptrs()
{
	long i, j, k, size;
	VPixel *data;

	if(total_in_buffers)
	{
		for(i = 0; i < total_in_buffers; i++)
		{
// get a single channel's double buffers
			input_ptr_master.append(new VFrame**[MAX_BUFFERS]);
			size = realtime_in_size.values[i];
			for(j = 0; j < double_buffers_in.values[i]; j++)
			{
// get a double buffer for the channel
				data = (VPixel*)data_in_realtime.values[i][j]->get_data();
				input_ptr_master.values[i][j] = new VFrame*[size];

// get the frames for the double buffer
				for(k = 0; k < size; k++)
				{
					input_ptr_master.values[i][j][k]
					 = new VFrame((unsigned char*)(&data[k * project_frame_w * project_frame_h]), project_frame_w, project_frame_h);
				}
			}
		}
		input_ptr_render = new VFrame**[total_in_buffers];
	}

	if(total_out_buffers)
	{
		for(i = 0; i < total_out_buffers; i++)
		{
			output_ptr_master.append(new VFrame**[MAX_BUFFERS]);
			size = realtime_out_size.values[i];
			for(j = 0; j < double_buffers_out.values[i]; j++)
			{
				data = (VPixel*)data_out_realtime.values[i][j]->get_data();
				output_ptr_master.values[i][j] = new VFrame*[size];
				for(k = 0; k < size; k++)
				{
					output_ptr_master.values[i][j][k]
					 = new VFrame((unsigned char*)(&data[k * project_frame_w * project_frame_h]), project_frame_w, project_frame_h);
				}
			}
		}
		output_ptr_render = new VFrame**[total_out_buffers];
	}
}

// Run before every realtime buffer is to be rendered.
int PluginVClient::get_render_ptrs()
{
	int i, j, double_buffer, fragment_position;

	for(i = 0; i < total_in_buffers; i++)
	{
		double_buffer = double_buffer_in_render.values[i];
		fragment_position = offset_in_render.values[i];
		input_ptr_render[i] = &input_ptr_master.values[i][double_buffer][fragment_position];
	}

	for(i = 0; i < total_out_buffers; i++)
	{
		double_buffer = double_buffer_out_render.values[i];
		fragment_position = offset_out_render.values[i];
		output_ptr_render[i] = &output_ptr_master.values[i][double_buffer][fragment_position];
	}
}

int PluginVClient::init_nonrealtime_parameters()
{
	long i, j, k, size;
	VPixel *data;

	project_frame_rate = get_project_framerate();
	get_project_framesize(project_frame_w, project_frame_h);
	use_float = get_use_float();
	use_alpha = get_use_alpha();
	use_interpolation = get_use_interpolation();

	if(total_in_buffers)
	{
// Get an array of channels
		video_in = new VFrame**[total_in_buffers];

		for(i = 0; i < total_in_buffers; i++)
		{
// Get an array of frames
			video_in[i] = new VFrame*[in_buffer_size];
			data = (VPixel*)data_in[i]->get_data();

// Usually this only allocates 1 frame per channel.
			for(j = 0; j < in_buffer_size; j++)
			{
// Get the frame.
				video_in[i][j] = new VFrame((unsigned char*)(data + j * project_frame_w * project_frame_h), project_frame_w, project_frame_h);
			}
// for(j = 0; j < project_frame_h; j++)
// {
// 	printf("%d %p\n", j, video_in[i][0]->get_rows()[j]);
// }
		}
	}
	
	if(total_out_buffers)
	{
// Get an array of channels
		video_out = new VFrame**[total_out_buffers];

		for(i = 0; i < total_out_buffers; i++)
		{
// Get an array of frames
			video_out[i] = new VFrame*[out_buffer_size];
			data = (VPixel*)data_out[i]->get_data();
			
			for(j = 0; j < out_buffer_size; j++)
			{
// Get the frame.
				video_out[i][j] = new VFrame((unsigned char*)(data + j * project_frame_w * project_frame_h), project_frame_w, project_frame_h);
			}
		}
	}
	return 0;
}

// Run after the non realtime plugin is run.
int PluginVClient::delete_nonrealtime_parameters()
{
	int i, j;

	for(i = 0; i < total_in_buffers; i++)
	{
		for(j = 0; j < in_buffer_size; j++)
		{
			delete video_in[i][j];
		}
	}

	for(i = 0; i < total_out_buffers; i++)
	{
		for(j = 0; j < out_buffer_size; j++)
		{
			delete video_out[i][j];
		}
	}
	video_in = 0;
	video_out = 0;

	return 0;
}

// Run after the realtime plugin is done.
int PluginVClient::delete_buffer_ptrs()
{
	int i, j, k, size;
	if(total_in_buffers)
	{
		for(i = 0; i < total_in_buffers; i++)
		{
			for(j = 0; j < double_buffers_in.values[i]; i++)
			{
// delete the frames from the double buffer
				size = realtime_in_size.values[i];
				for(k = 0; k < size; k++)
				{
					delete input_ptr_master.values[i][j][k];
				}
// delete the double buffer from the channel
				delete input_ptr_master.values[i][j];
			}
// delete array of double buffers
			delete input_ptr_master.values[i];
		}
// delete all the channels
		input_ptr_master.remove_all();
		delete input_ptr_render;
	}

	if(total_out_buffers)
	{
		for(i = 0; i < total_out_buffers; i++)
		{
			for(j = 0; j < double_buffers_out.values[i]; i++)
			{
// delete the frames from the double buffer
				size = realtime_out_size.values[i];
				for(k = 0; k < size; k++)
				{
					delete output_ptr_master.values[i][j][k];
				}
// delete the double buffer from the channel
				delete output_ptr_master.values[i][j];
			}
			delete output_ptr_master.values[i];
		}
// delete all the channels
		output_ptr_master.remove_all();
		delete output_ptr_render;
	}
}

int PluginVClient::init_realtime_parameters()
{
	project_frame_rate = get_project_framerate();
	get_project_framesize(project_frame_w, project_frame_h);
	use_float = get_use_float();
	use_alpha = get_use_alpha();
	use_interpolation = get_use_interpolation();
	get_aspect_ratio(aspect_w, aspect_h);
}


int PluginVClient::process_realtime(long size)
{
	get_render_ptrs();
	
	if(plugin_is_multi_channel())
		process_realtime(size, input_ptr_render, output_ptr_render);
	else
		process_realtime(size, input_ptr_render[0], output_ptr_render[0]);
}

int PluginVClient::process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr)
{
	printf("process_realtime(long size, VFrame ***input_ptr, VFrame ***output_ptr) not defined.\n");
}
// realtime process for a single channel plugin
int PluginVClient::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	printf("process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr) not defined.\n");
}
