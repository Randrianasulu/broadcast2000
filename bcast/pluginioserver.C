#include <string.h>
#include "assets.h"
#include "messages.h"
#include "pluginbuffer.h"
#include "plugincommands.h"
#include "pluginioserver.h"
#include "strategies.inc"
#include "vframe.h"

// ================================ File I/O

PluginIOServer::PluginIOServer() : PluginServer()
{
}

PluginIOServer::PluginIOServer(PluginServer &server)
 : PluginServer(server)
{
}

PluginIOServer::~PluginIOServer()
{
}


int PluginIOServer::get_audio_parameters()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_AUDIO_PARAMETERS);
	int result = messages->read_message();
	return result;
return 0;
}

int PluginIOServer::get_video_parameters()
{
	if(!plugin_open) return 0;
	messages->write_message(GET_VIDEO_PARAMETERS);
	int result = messages->read_message();
	return result;
return 0;
}

int PluginIOServer::interrupt_audio_parameters()
{
	if(!plugin_open) return 0;
//	messages->write_message(INTERRUPT_AUDIO_PARAMETERS);
return 0;
}

int PluginIOServer::interrupt_video_parameters()
{
	if(!plugin_open) return 0;
//	messages->write_message(INTERRUPT_VIDEO_PARAMETERS);
return 0;
}

int PluginIOServer::check_header(char *path)
{
	if(!plugin_open) return 0;
	messages->write_message(CHECK_HEADER);
	messages->write_message(path);
	plugin_server_loop();
	int result = messages->read_message();
	return result;
return 0;
}

int PluginIOServer::set_cpus(int cpus)
{
	if(!plugin_open) return 0;
	messages->write_message(SET_CPUS);
	messages->write_message(cpus);
	return 0;
return 0;
}

int PluginIOServer::open_file(Asset *asset, int rd, int wr)
{
	if(!plugin_open) return 0;
	messages->write_message(OPEN_FILE);
	messages->write_message(asset->path);
	messages->write_message(rd, wr);

	if(wr) send_header(asset);
	if(rd) get_header(asset);

	int result = messages->read_message();

	return result;
return 0;
}

int PluginIOServer::close_file()
{
	if(!plugin_open) return 0;
	messages->write_message(CLOSE_FILE);
	return 0;
return 0;
}

int PluginIOServer::send_header(Asset *asset)
{
	messages->write_message(asset->audio_data);
	if(asset->audio_data)
	{
		messages->write_message(asset->rate);
		messages->write_message(asset->bits);
		messages->write_message(asset->byte_order);
		messages->write_message(asset->signed_);
		messages->write_message(asset->header);
		messages->write_message(asset->channels);
	}

	messages->write_message(asset->video_data);
	if(asset->video_data)
	{
		messages->write_message(asset->layers);
		messages->write_message_f(asset->frame_rate);
		messages->write_message(asset->width);
		messages->write_message(asset->height);
		messages->write_message(asset->quality);
		messages->write_message(asset->compression);
	}
return 0;
}

int PluginIOServer::get_header(Asset *asset)
{
	if(!plugin_open) return 0;
	asset->audio_data = messages->read_message();
	if(asset->audio_data)
	{
		asset->rate = messages->read_message();
		asset->bits = messages->read_message();
		asset->byte_order = messages->read_message();
		asset->signed_ = messages->read_message();
		asset->header = messages->read_message();
		asset->channels = messages->read_message();
	}

	asset->video_data = messages->read_message();
	if(asset->video_data)
	{
		asset->layers = messages->read_message();

// Don't want a user configured frame rate to get destroyed during an import
		if(!asset->frame_rate) 
			asset->frame_rate = messages->read_message_f();
		else
			messages->read_message_f();

		asset->width = messages->read_message();
		asset->height = messages->read_message();
		asset->quality = messages->read_message();
		messages->read_message(asset->compression);
	}

	return 0;
return 0;
}

long PluginIOServer::get_audio_length()
{
	if(!plugin_open) return 0;
	messages->write_message(FILE_GET_ALENGTH);
	return messages->read_message();
}

long PluginIOServer::get_video_length()
{
	if(!plugin_open) return -1;
	messages->write_message(FILE_GET_VLENGTH);
	return messages->read_message();
}

int PluginIOServer::seek_end()
{
	if(!plugin_open) return -1;
	messages->write_message(FILE_SEEK_END);
	return messages->read_message();
return 0;
}

int PluginIOServer::seek_start()
{
	if(!plugin_open) return -1;
	messages->write_message(FILE_SEEK_START);
	return messages->read_message();
return 0;
}

long PluginIOServer::get_video_position()
{
	if(!plugin_open) return -1;
	messages->write_message(FILE_GET_VIDEO_POSITION);
	return messages->read_message();
}

long PluginIOServer::get_audio_position()
{
	if(!plugin_open) return -1;
	messages->write_message(FILE_GET_AUDIO_POSITION);
	return messages->read_message();
}

int PluginIOServer::set_video_position(long frame)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_SET_VIDEO_POSITION);
	messages->write_message(frame);
	return messages->read_message();
return 0;
}

int PluginIOServer::set_audio_position(long sample)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_SET_AUDIO_POSITION);
	messages->write_message(sample);
	return messages->read_message();
return 0;
}

int PluginIOServer::set_channel(int channel)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_SET_CHANNEL);
	messages->write_message(channel);
	return messages->read_message();
return 0;
}

int PluginIOServer::set_layer(int layer)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_SET_LAYER);
	messages->write_message(layer);
	return messages->read_message();
return 0;
}

int PluginIOServer::read_samples(PluginBuffer *buffer, long offset, long len, int feather, 
	long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_READ_SAMPLES);
	messages->write_message(buffer->get_id(), buffer->get_size(), offset, len);
	messages->write_message(feather, lfeather_len);
	messages->write_message_f(lfeather_gain, lfeather_slope);
	return messages->read_message();
return 0;
}


int PluginIOServer::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
	float in_x1, float in_y1, float in_x2, float in_y2,
	float out_x1, float out_y1, float out_x2, float out_y2, 
	long alpha, long use_alpha, long use_float, long interpolate)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_READ_FRAME);
	messages->write_message(buffer->get_id(), buffer->get_size(), byte_offset);
	messages->write_message_f(in_x1, in_y1, in_x2, in_y2);
	messages->write_message_f(out_x1, out_y1, out_x2, out_y2);
	messages->write_message(alpha, use_alpha, use_float, interpolate);
	messages->write_message(frame->get_w(), frame->get_h());
	return messages->read_message();
return 0;
}

VFrame* PluginIOServer::read_frame(int use_alpha, int use_float)
{
	long shared_id, shared_size;
	long width, height;
	int result;

	if(!plugin_open) return 0;
	messages->write_message(FILE_READ_FRAME_PTR);
	messages->write_message(use_alpha, use_float);
	messages->read_message(&width, &height, &shared_id, &shared_size);
	
	if(!temp_frame)
	{
		temp_frame_buffer = new PluginBuffer(shared_id, shared_size, sizeof(VPixel));
		temp_frame = new VFrame((unsigned char*)temp_frame_buffer->get_data(), width, height, VFRAME_VPIXEL);
	}
	
	result = messages->read_message();
	return temp_frame;
}

int PluginIOServer::read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_READ_RAW_FRAME);
	messages->write_message(buffer->get_id(), buffer->get_size(), byte_offset, frame->get_color_model());
	return messages->read_message();
return 0;
}


int PluginIOServer::read_raw_frame_possible()
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_RAW_FRAME_POSSIBLE);
	return messages->read_message();
return 0;
}

int PluginIOServer::write_frame(VFrame *frame, 
			PluginBuffer *video_ram, 
			long byte_offset, 
			int use_alpha, 
			int use_float)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_WRITE_FRAME);
	messages->write_message(video_ram->get_id(), byte_offset, use_alpha, use_float);
	messages->write_message(frame->get_w(), frame->get_h(), frame->get_color_model());
	return messages->read_message();
return 0;
}

int PluginIOServer::write_samples(float **buffer, 
			PluginBuffer *audio_ram, 
			long byte_offset, 
			long allocated_samples, 
			long len)
{
	if(!plugin_open) return 1;
	messages->write_message(FILE_WRITE_SAMPLES);
	messages->write_message(audio_ram->get_id(), byte_offset, allocated_samples, len);
	return messages->read_message();
return 0;
}
			
int PluginIOServer::test_render_strategy(int render_strategy)
{
	if(!plugin_open) return 1;
	int result;

	if(render_strategy == VRENDER_VPIXEL) return 1;

	messages->write_message(FILE_STRATEGY_POSSIBLE);
	messages->write_message(render_strategy);
	result = messages->read_message();
	return result;
return 0;
}


