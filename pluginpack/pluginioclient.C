#include <string.h>
#include "pluginioclient.h"
#include "vframe.h"


PluginIOClient::PluginIOClient(int argc, char *argv[])
 : PluginClient(argc, argv)
{
	compression[0] = 0;
	cpus = 1;
}

PluginIOClient::~PluginIOClient()
{
}

int PluginIOClient::plugin_is_fileio()
{
	return 1;
}

int PluginIOClient::plugin_command_derived(int plugin_command)
{
	switch(plugin_command)
	{
		case OPEN_FILE:               plugin_open_file();		    	break;
		case CLOSE_FILE:              plugin_close_file();  	    	break;
		case CHECK_HEADER:            plugin_check_header();	    	break;
		case GET_AUDIO_PARAMETERS:    plugin_get_aparameters();     	break;
		case GET_VIDEO_PARAMETERS:    plugin_get_vparameters();     	break;
		case FILE_GET_ALENGTH:        plugin_get_alength(); 	    	break;
		case FILE_GET_VLENGTH:        plugin_get_vlength(); 	    	break;
		case FILE_SEEK_END:           plugin_seek_end();                break;
		case FILE_SEEK_START:         plugin_seek_start();              break;
		case FILE_GET_VIDEO_POSITION: plugin_get_vposition();           break;
		case FILE_GET_AUDIO_POSITION: plugin_get_aposition();           break;
		case FILE_SET_VIDEO_POSITION: plugin_set_vposition();           break;
		case FILE_SET_AUDIO_POSITION: plugin_set_aposition();           break;
		case FILE_SET_CHANNEL:        plugin_set_channel();             break;
		case FILE_SET_LAYER:          plugin_set_layer();               break;
		case FILE_READ_SAMPLES:       plugin_read_samples();            break;
		case FILE_READ_FRAME:         plugin_read_frame();              break;
		case FILE_READ_FRAME_PTR:     plugin_read_frame_ptr();          break;
		case FILE_READ_RAW_FRAME:     plugin_read_raw_frame();          break;
		case SET_CPUS:                plugin_set_cpus();                break;
		case FILE_STRATEGY_POSSIBLE:  plugin_test_strategy();           break;
		case FILE_RAW_FRAME_POSSIBLE: plugin_raw_frame_possible();      break;
		case FILE_WRITE_SAMPLES:      plugin_write_samples();			break;
		case FILE_WRITE_FRAME:        plugin_write_frame();  			break;
	}
	return 0;
}

int PluginIOClient::plugin_check_header()
{
	char path[1024];

	messages->read_message(path);
	int result = check_header(path);
	send_completed();
	messages->write_message(result);
	return 0;
}

int PluginIOClient::plugin_set_cpus()
{
	this->cpus = messages->read_message();
	return 0;
}

int PluginIOClient::plugin_open_file()
{
	char path[1024];
	long rd_l, wr_l;
	messages->read_message(path);
	messages->read_message(&rd_l, &wr_l);
	wr = wr_l;
	rd = rd_l;

// Reset header
	has_audio = 0;
	sample_rate = 0;
	channels = 0;
	bits = 0;
	byteorder = 0;
	signed_ = 0;
	header = 0;

	has_video = 0;
	layers = 0;
	frame_rate = 0;
	width = 0;
	height = 0;
	quality = 0;
	compression[0] = 0;

// Get header
	if(wr)
	{
		get_header();
	}

// Open it for real
	int result = open_file(path, rd, wr);

	if(rd)
	{
		send_header();
	}
	
	messages->write_message(result);
return 0;
}

int PluginIOClient::send_header()
{
	messages->write_message(has_audio);
	if(has_audio)
	{
		messages->write_message(sample_rate);
		messages->write_message(bits);
		messages->write_message(byteorder);
		messages->write_message(signed_);
		messages->write_message(header);
		messages->write_message(channels);
	}

	messages->write_message(has_video);
	if(has_video)
	{
		messages->write_message(layers);
		messages->write_message_f(frame_rate);
		messages->write_message(width);
		messages->write_message(height);
		messages->write_message(quality);
		messages->write_message(compression);
	}
	return 0;
}

int PluginIOClient::get_header()
{
	messages->write_message(has_audio);
	if(has_audio)
	{
		sample_rate = messages->read_message();
		bits = messages->read_message();
		byteorder = messages->read_message();
		signed_ = messages->read_message();
		header = messages->read_message();
		channels = messages->read_message();
	}

	messages->write_message(has_video);
	if(has_video)
	{
		layers = messages->read_message();
		frame_rate = messages->read_message_f();
		width = messages->read_message();
		height = messages->read_message();
		quality = messages->read_message();
		messages->read_message(compression);
	}
	return 0;
}

int PluginIOClient::plugin_close_file()
{
	close_file();
return 0;
}

int PluginIOClient::plugin_get_aparameters()
{
	int result = get_audio_parameters();
return 0;
}

int PluginIOClient::plugin_get_vparameters()
{
	int result = get_video_parameters();
return 0;
}

int PluginIOClient::plugin_interrupt_aparameters() { return interrupt_aparameters(); }
int PluginIOClient::plugin_interrupt_vparameters() { return interrupt_vparameters(); }
long PluginIOClient::plugin_get_alength() { messages->write_message(get_alength()); return 0;}
long PluginIOClient::plugin_get_vlength() { messages->write_message(get_vlength()); return 0;}
int PluginIOClient::plugin_seek_end() { messages->write_message(seek_end()); return 0; }
int PluginIOClient::plugin_seek_start() { messages->write_message(seek_start()); return 0; }
long PluginIOClient::plugin_get_vposition() { messages->write_message(get_vposition()); return 0;}
long PluginIOClient::plugin_get_aposition() { messages->write_message(get_aposition()); return 0; }
int PluginIOClient::plugin_set_vposition() { messages->write_message(set_vposition(messages->read_message())); return 0;}
int PluginIOClient::plugin_set_aposition() { messages->write_message(set_aposition(messages->read_message())); return 0;}
int PluginIOClient::plugin_set_channel() { messages->write_message(set_channel(messages->read_message())); return 0;}
int PluginIOClient::plugin_set_layer() { messages->write_message(set_layer(messages->read_message())); return 0;}

int PluginIOClient::plugin_write_samples()
{
	long shm_id, byte_offset, allocated_samples, len;
	float **buffer;
	
	buffer = new float*[channels];

	messages->read_message(&shm_id, &byte_offset, &allocated_samples, &len);
	PluginBuffer audio_ram(shm_id, 0, 0);
	for(int i = 0; i < channels; i++)
	{
		buffer[i] = (float*)((unsigned char*)audio_ram.get_data() + byte_offset) + i * allocated_samples;
	}

	int result = write_samples(buffer, len);

	delete [] buffer;
	return 0;
}

int PluginIOClient::plugin_write_frame()
{
	long shm_id, byte_offset, use_alpha, use_float;
	long frame_w, frame_h, color_model;

	messages->read_message(&shm_id, &byte_offset, &use_alpha, &use_float);
	messages->read_message(&frame_w, &frame_h, &color_model);

	PluginBuffer buffer(shm_id, 0, 0);
	VFrame frame(((unsigned char*)buffer.get_data() + byte_offset), 
				frame_w, 
				frame_h, 
				color_model);
	int result = write_frame(&frame, 
				use_alpha, 
				use_float);
	return result;
}

int PluginIOClient::plugin_read_samples()
{
	int result = 0;
	long shared_id;
	long shared_size;
	long shared_offset;
	PluginBuffer *shared_buffer;
	float *buffer;
	long len;
	long feather;
	long lfeather_len;
	float lfeather_gain;
	float lfeather_slope; 

	messages->read_message(&shared_id, &shared_size, &shared_offset, &len);
	messages->read_message(&feather, &lfeather_len);
	messages->read_message_f(&lfeather_gain, &lfeather_slope);

	shared_buffer = new PluginBuffer(shared_id, shared_size, sizeof(float));
	buffer = (float*)shared_buffer->get_data() + shared_offset;
	result = read_samples(buffer, len, 
		feather, 
		lfeather_len, lfeather_gain, lfeather_slope);

	delete shared_buffer;
	messages->write_message(result);
	return 0;
}

int PluginIOClient::plugin_read_frame()
{
	long shared_id, shared_size, shared_offset;
	float in_x1, in_y1, in_x2, in_y2;
	float out_x1, out_y1, out_x2, out_y2;
	long alpha, use_alpha, use_float, interpolate;
	long frame_w, frame_h;

	messages->read_message(&shared_id, &shared_size, &shared_offset);
	messages->read_message_f(&in_x1, &in_y1, &in_x2, &in_y2);
	messages->read_message_f(&out_x1, &out_y1, &out_x2, &out_y2);
	messages->read_message(&alpha, &use_alpha, &use_float, &interpolate);
	messages->read_message(&frame_w, &frame_h);

	PluginBuffer buffer(shared_id, 0, 0);
	VFrame frame(((unsigned char*)buffer.get_data() + shared_offset), 
				frame_w, 
				frame_h, 
				VFRAME_VPIXEL);
	int result = read_frame(&frame, 
				in_x1, in_y1, in_x2, in_y2,
				out_x1, out_y1, out_x2, out_y2, 
				alpha, use_alpha, use_float, interpolate);

	messages->write_message(result);
	return 0;
}

int PluginIOClient::plugin_read_frame_ptr()
{
	long use_alpha, use_float;
	long shared_id, shared_size;
	int result;
	messages->read_message(&use_alpha, &use_float);

	result = read_frame_ptr(use_alpha, use_float, shared_id, shared_size);
	messages->write_message(width, height, shared_id, shared_size);
	messages->write_message(result);
	return 0;
}

int PluginIOClient::plugin_read_raw_frame()
{
	long shared_id, shared_size, shared_offset, color_model;

	messages->read_message(&shared_id, &shared_size, &shared_offset, &color_model);

	PluginBuffer buffer(shared_id, shared_size, VFrame::bytes_per_pixel(color_model));
	VFrame frame((unsigned char*)buffer.get_data() + shared_offset, width, height, color_model);
	int result = read_raw_frame(&frame);
	messages->write_message(result);
	return 0;
}

int PluginIOClient::plugin_raw_frame_possible()
{
	messages->write_message(read_raw_frame_possible());
	return 0;
}

int PluginIOClient::plugin_test_strategy()
{
	int strategy = messages->read_message();
	int result = messages->write_message(test_strategy(strategy));
	return result;
}
