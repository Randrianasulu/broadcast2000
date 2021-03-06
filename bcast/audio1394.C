#include <string.h>
#include "audio1394.h"
#include "audioconfig.h"
#include "videoconfig.h"
#include "videodevice.h"

#ifdef HAVE_FIREWIRE
#define SAMPLES_PER_FRAME 2048

Audio1394::Audio1394(AudioDevice *device) : AudioLowLevel(device)
{
	initialize();
}


Audio1394::~Audio1394()
{
}

int Audio1394::initialize()
{
	dv = 0;
	grabber = 0;
	ring_buffer = 0;
	ring_buffer_size = 0;
return 0;
}

int Audio1394::interrupt_crash()
{
	if(dv_grabber_crashed(grabber))
		dv_interrupt_grabber(grabber);
return 0;
}

int Audio1394::open_input()
{
	dv = dv_new();
	grabber = dv_grabber_new();

// Fix the ichannels for the DV format
	device->in_channels = 2;
	device->in_bits = 16;
	frames = 30;

	if(dv_start_grabbing(grabber, 
			device->config->afirewire_in_port, 
			device->config->afirewire_in_channel, 
			frames))
	{
		dv_grabber_delete(grabber);
		grabber = 0;
	}
	else
	{
		bytes_per_sample = 4;
		ring_buffer = new unsigned char[2 * frames * 2048 * bytes_per_sample];
	}

	return 0;
return 0;
}

int Audio1394::close_all()
{
	if(device->sharing)
	{
		device->vdevice->stop_sharing();
	}

	if(dv)
	{
		if(grabber)
		{
			dv_stop_grabbing(grabber);
			dv_grabber_delete(grabber);
		}
		dv_delete(dv);
	}
	initialize();
	return 0;
return 0;
}


int Audio1394::read_buffer(char *buffer, long bytes)
{
	int result = 0;
	unsigned char *data;
	long data_size;
	int samples_recieved;  
	long samples = bytes / bytes_per_sample;
	int i, j;

	if(!grabber) return 1;

	if(device->vdevice &&
		device->vdevice->in_config->video_in_driver == CAPTURE_FIREWIRE)
		device->sharing = 1;

// Read frames until the ring buffer is bigger than bytes
	while(!result && ring_buffer_size < bytes)
	{
// Get the frame
		result = dv_grab_frame(grabber, &data, &data_size);
// Pass it on to the video device
		if(!result && device->sharing)
			device->vdevice->get_shared_data(data, data_size);

// Extract the audio
		if(!result)
			samples_recieved = 
				dv_read_audio(dv, 
					ring_buffer + ring_buffer_size,
					data, 
					data_size);

		dv_unlock_frame(grabber);
		if(!result) ring_buffer_size += samples_recieved * bytes_per_sample;
	}

	if(!result)
	{
		for(i = 0; i < bytes; i++)
		{
			buffer[i] = ring_buffer[i];
		}

		for(i = bytes, j = 0; 
			i < ring_buffer_size; 
			i++, j++)
		{
			ring_buffer[j] = ring_buffer[i];
		}

		ring_buffer_size -= bytes;
	}

	return result;
return 0;
}


#endif
