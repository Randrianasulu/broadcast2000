#include <string.h>
#include "pluginaclient.h"



PluginAClient::PluginAClient(int argc, char *argv[])
 : PluginClient(argc, argv)
{
}

PluginAClient::~PluginAClient()
{
}

int PluginAClient::plugin_is_audio()
{
	return 1;
}

int PluginAClient::create_buffer_ptrs()
{
	int i, j;
	
	for(i = 0; i < total_in_buffers; i++)
	{
		input_ptr_master.append(new float*[MAX_BUFFERS]);
//printf("double_buffers_in.values[i] %d\n", double_buffers_in.values[i]);
		for(j = 0; j < double_buffers_in.values[i]; j++)
		{
			input_ptr_master.values[i][j] = (float*)data_in_realtime.values[i][j]->get_data();
//printf("input_ptr_master.values[%d][%d] = %x\n", i, j, input_ptr_master.values[i][j]);
		}
	}
	input_ptr_render = new float*[total_in_buffers];

	for(i = 0; i < total_out_buffers; i++)
	{
		output_ptr_master.append(new float*[MAX_BUFFERS]);
		for(j = 0; j < double_buffers_out.values[i]; j++)
		{
			output_ptr_master.values[i][j] = (float*)data_out_realtime.values[i][j]->get_data();
		}
	}
	
	output_ptr_render = new float*[total_out_buffers];
}

int PluginAClient::delete_buffer_ptrs()
{
	int i;
// delete double buffer arrays
	for(int i = 0; i < total_in_buffers; i++)
	{
		delete input_ptr_master.values[i];
	}
	for(int i = 0; i < total_out_buffers; i++)
	{
		delete output_ptr_master.values[i];
	}
	input_ptr_master.remove_all();
	output_ptr_master.remove_all();
	delete input_ptr_render;
	delete output_ptr_render;
}

int PluginAClient::get_render_ptrs()
{
	int i, j, double_buffer, fragment_position;

	for(i = 0; i < total_in_buffers; i++)
	{
		double_buffer = double_buffer_in_render.values[i];
		fragment_position = offset_in_render.values[i];
		input_ptr_render[i] = &input_ptr_master.values[i][double_buffer][fragment_position];
//printf("PluginAClient::get_render_ptrs %x\n", input_ptr_master.values[i][double_buffer]);
	}

	for(i = 0; i < total_out_buffers; i++)
	{
		double_buffer = double_buffer_out_render.values[i];
		fragment_position = offset_out_render.values[i];
		output_ptr_render[i] = &output_ptr_master.values[i][double_buffer][fragment_position];
	}
//printf("PluginAClient::get_render_ptrs %x %x\n", input_ptr_render[0], output_ptr_render[0]);
}

int PluginAClient::init_realtime_parameters()
{
	project_sample_rate = get_project_samplerate();	
}

int PluginAClient::process_realtime(long size)
{
	get_render_ptrs();
	
	if(plugin_is_multi_channel())
		process_realtime(size, input_ptr_render, output_ptr_render);
	else
		process_realtime(size, input_ptr_render[0], output_ptr_render[0]);
}
