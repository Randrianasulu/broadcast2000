#include "audioconfig.h"
#include "audiodevice.h"
#include "audioesound.h"

#ifdef HAVE_ESOUND
#include <esd.h>
#include <string.h>

AudioESound::AudioESound(AudioDevice *device) : AudioLowLevel(device)
{
	esd_in = esd_out = esd_duplex = 0;
	esd_in_fd = esd_out_fd = esd_duplex_fd = 0;
}

AudioESound::~AudioESound()
{
}


int AudioESound::get_bit_flag(int bits)
{
	switch(bits)
	{
		case 8:
			return ESD_BITS8;
			break;
		
		case 16:
			return ESD_BITS16;
			break;
		
		case 24:
			return ESD_BITS16;
			break;
	}
return 0;
}

// No more than 2 channels in ESD
int AudioESound::get_channels_flag(int channels)
{
	switch(channels)
	{
		case 1:
			return ESD_MONO;
			break;
		
		case 2:
			return ESD_STEREO;
			break;
		
		default:
			return ESD_STEREO;
			break;
	}
return 0;
}

char* AudioESound::translate_device_string(char *server, int port)
{
// ESD server
	if(port > 0 && strlen(server))
		sprintf(device_string, "%s:%d", server, port);
	else
		sprintf(device_string, "");
	return device_string;
}

int AudioESound::open_input()
{
	esd_format_t format = ESD_STREAM | ESD_RECORD;
	
	device->in_channels = 2;
	device->in_bits = 16;

	format |= get_channels_flag(device->in_channels);
	format |= get_bit_flag(device->in_bits);

	if((esd_in = esd_open_sound(translate_device_string(device->config->esound_in_server, device->config->esound_in_port))) <= 0)
	{
		fprintf(stderr, "AudioESound::open_input: open failed\n");
		return 1;
	}
	esd_in_fd = esd_record_stream_fallback(format, device->in_samplerate, 
			    	translate_device_string(device->config->esound_out_server, device->config->esound_out_port), 
						"Bcast 2000");
	return 0;
return 0;
}

int AudioESound::open_output()
{
	esd_format_t format = ESD_STREAM | ESD_PLAY;

	device->out_channels = 2;
	device->out_bits = 16;

	format |= get_channels_flag(device->out_channels);
	format |= get_bit_flag(device->out_bits);

	if((esd_out = esd_open_sound(translate_device_string(device->config->esound_out_server, device->config->esound_out_port))) <= 0)
	{
		fprintf(stderr, "AudioESound::open_output: open failed\n");
		return 1;
	};
	esd_out_fd = esd_play_stream_fallback(format, device->out_samplerate, 
			    	translate_device_string(device->config->esound_out_server, device->config->esound_out_port), 
						"Bcast 2000");

	device->device_buffer = esd_get_latency(esd_out);
	device->device_buffer *= device->out_bits / 8 * device->out_channels;
	return 0;
return 0;
}

// Open both input and output in ESD
int AudioESound::open_duplex()
{
	device->duplex_channels = 2;
	device->duplex_bits = 16;
	open_input();
	open_output();
	return 0;
return 0;
}

int AudioESound::close_all()
{
	if(device->r || device->d)
	{ 
    	close(esd_in_fd);
		esd_close(esd_in);      
	}

	if(device->w || device->d)
	{
 	   	close(esd_out_fd);
		esd_close(esd_out);     
	}
return 0;
}

// No position on ESD
long AudioESound::device_position()
{
	return -1;
}

int AudioESound::read_buffer(char *buffer, long size)
{
	if(esd_in_fd > 0)
		return read(esd_in_fd, buffer, size);
	else
		return 1;
return 0;
}

int AudioESound::write_buffer(char *buffer, long size)
{
	if(esd_out_fd > 0)
		return write(esd_out_fd, buffer, size);
	else
		return 0;
return 0;
}

// No flushing in ESD
int AudioESound::flush_device()
{
	return 0;
return 0;
}

// No interrupting ESD
int AudioESound::interrupt_playback()
{
	return 0;
return 0;
}

#endif // HAVE_ESOUND
