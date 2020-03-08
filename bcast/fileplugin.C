#include <string.h>
#include "assets.h"
#include "file.h"
#include "fileplugin.h"
#include "pluginioserver.h"
#include "vframe.h"



FilePlugin::FilePlugin(ArrayList<PluginServer*> *plugindb, 
	Asset *asset, 
	File *file)
 : FileBase(asset, file)
{
	reset_parameters();
	this->plugindb = plugindb;
}

FilePlugin::~FilePlugin()
{
	close_file();
}

int FilePlugin::reset_parameters_derived()
{
	plugin = 0;
	prev_track = 0;
	data = 0;
return 0;
}

int FilePlugin::check_header()
{
	int i;
	int result = 0;

	for(i = 0; i < plugindb->total && !result; i++)
	{
		if(plugindb->values[i]->fileio)
		{
			PluginIOServer plugin(*(plugindb->values[i]));
			plugin.open_plugin();
			result = plugin.check_header(asset->path);
			plugin.close_plugin();
			if(result)
			{
				asset->format = i | 0x8000;
				return 1;
			}
		}
	}
	return 0;
return 0;
}

int FilePlugin::open_file(int rd, int wr)
{
	int result = 1;
	if(asset->format & 0x8000)
	{
		if((asset->format & 0x7fff) < plugindb->total)
		{
			plugin = new PluginIOServer(*(plugindb->values[asset->format & 0x7fff]));
			plugin->open_plugin();
			plugin->set_cpus(file->cpus);
			result = plugin->open_file(asset, rd, wr);
			if(result)
			{
				plugin->close_plugin();
				delete plugin;
				plugin = 0;
			}
		}
	}
	return result;
return 0;
}

int FilePlugin::close_file_derived()
{
	if(plugin)
	{
		plugin->close_file();
		plugin->close_plugin();
		delete plugin;
	}

	if(data)
	{
		delete data;
	}

	data = 0;
	plugin = 0;
return 0;
}

long FilePlugin::get_audio_length()
{
	return plugin->get_audio_length();
}

long FilePlugin::get_video_length()
{
	return plugin->get_video_length();
}

int FilePlugin::seek_end()
{
	return plugin->seek_end();
return 0;
}

int FilePlugin::seek_start()
{
	return plugin->seek_start();
return 0;
}

long FilePlugin::get_video_position()
{
	return plugin->get_video_position();
}

long FilePlugin::get_audio_position()
{
	return plugin->get_audio_position();
}

int FilePlugin::set_audio_position(long x)
{
	return plugin->set_audio_position(x);
return 0;
}

int FilePlugin::set_video_position(long x)
{
	return plugin->set_video_position(x);
return 0;
}

int FilePlugin::set_channel(int channel)
{
	return plugin->set_channel(channel);
return 0;
}

int FilePlugin::set_layer(int layer)
{
	return plugin->set_layer(layer);
return 0;
}

int FilePlugin::write_samples(float **buffer, 
		PluginBuffer *audio_ram, 
		long byte_offset, 
		long allocated_samples, 
		long len)
{
	int result = plugin->write_samples(buffer, audio_ram, byte_offset, allocated_samples, len);
	return result;
return 0;
}

int FilePlugin::write_frames(VFrame ***frames, 
		PluginBuffer *video_ram, 
		int len, 
		int use_alpha, 
		int use_float)
{
	int result = 0;
	for(int j = 0; j < file->asset->layers && !result; j++)
		for(int i = 0; i < len && !result; i++)
		{
			result = plugin->write_frame(frames[j][i], 
				video_ram, 
				frames[j][i]->get_shm_offset(), 
				use_alpha, 
				use_float);
		}
	return result;
return 0;
}

int FilePlugin::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	int result = plugin->read_frame(frame, buffer, byte_offset, 
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha, use_alpha, use_float, interpolate);
	return result;
return 0;
}

VFrame* FilePlugin::read_frame(int use_alpha, int use_float)
{
	VFrame *result = plugin->read_frame(use_alpha, use_float);
	return result;
}

int FilePlugin::read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset)
{
	int result = plugin->read_raw_frame(frame, buffer, byte_offset);
	return result;
return 0;
}



int FilePlugin::read_samples(PluginBuffer *buffer, long offset, long len, 
		int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	int result = plugin->read_samples(buffer, offset, len, feather,
		lfeather_len, lfeather_gain, lfeather_slope);
	return result;
return 0;
}

long FilePlugin::get_memory_usage()
{
	return 1;
}


int FilePlugin::read_raw_frame_possible()
{
	int result = plugin->read_raw_frame_possible();
	return result;
return 0;
}

int FilePlugin::get_render_strategy(ArrayList<int>* render_strategies)
{
	if(search_render_strategies(render_strategies, VRENDER_RGB888) &&
		plugin->test_render_strategy(VRENDER_RGB888))
		return VRENDER_RGB888;

	return VRENDER_VPIXEL;
return 0;
}
