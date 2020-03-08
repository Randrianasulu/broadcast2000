#include <string.h>
#include "assets.h"
#include "audioconfig.h"
#include "audiodevice.h"
#include "file.inc"
#include "vdevice1394.h"
#include "vframe.h"
#include "videoconfig.h"
#include "videodevice.h"

#ifdef HAVE_FIREWIRE

VDevice1394::VDevice1394(VideoDevice *device)
 : VDeviceBase(device)
{
	initialize();
}

VDevice1394::~VDevice1394()
{
	if(grabber) dv_grabber_delete(grabber);
}

int VDevice1394::initialize()
{
	grabber = 0;
	shared_frames = 0;
return 0;
}

int VDevice1394::open_input()
{
// Initialize sharing
	if(device->adevice &&
		device->adevice->config->audio_in_driver == AUDIO_1394)
	{
		device->sharing = 1;
		shared_output_number = shared_input_number = 0;
		shared_ready = new int[device->in_config->capture_length];

		shared_output_lock = new Mutex[device->in_config->capture_length];
		shared_frames = new VFrame*[device->in_config->capture_length];
		for(int i = 0; i < device->in_config->capture_length; i++)
		{
			shared_frames[i] = new VFrame;
			shared_output_lock[i].lock();
			shared_ready[i] = 1;
		}
	}
	else
// Initialize grabbing
	{
		grabber = dv_grabber_new();
		if(dv_start_grabbing(grabber, 
			device->in_config->vfirewire_in_port, 
			device->in_config->vfirewire_in_channel, 
			device->in_config->capture_length))
		{
			dv_grabber_delete(grabber);
			grabber = 0;
			return 1;
		}
	}
	return 0;
return 0;
}

int VDevice1394::open_output()
{
	return 1;
return 0;
}

int VDevice1394::interrupt_crash()
{
	if(grabber && dv_grabber_crashed(grabber))
		dv_interrupt_grabber(grabber);
return 0;
}

int VDevice1394::close_all()
{
	if(device->sharing)
	{
		for(int i = 0; i < device->in_config->capture_length; i++)
		{
			delete shared_frames[i];
		}
		delete [] shared_frames;
		delete [] shared_output_lock;
		delete [] shared_ready;
	}
	else
	if(grabber)
	{
// Interrupt the video device
		if(device->get_failed())
			dv_interrupt_grabber(grabber);

		dv_stop_grabbing(grabber);
		dv_grabber_delete(grabber);
	}
	initialize();
	return 0;
return 0;
}

int VDevice1394::get_shared_data(unsigned char *data, long size)
{
	if(device->sharing)
		if(shared_ready[shared_input_number])
		{
			shared_ready[shared_input_number] = 0;
			shared_frames[shared_input_number]->allocate_compressed_data(size);
			memcpy(shared_frames[shared_input_number]->get_data(), data, size);
			shared_frames[shared_input_number]->set_compressed_size(size);
			shared_output_lock[shared_input_number].unlock();
			shared_input_number++;
			if(shared_input_number >= device->in_config->capture_length) shared_input_number = 0;
		}
	return 0;
return 0;
}

int VDevice1394::stop_sharing()
{
	for(int i = 0; i < device->in_config->capture_length; i++)
		shared_output_lock[i].unlock();
	return 0;
return 0;
}

int VDevice1394::read_buffer(VFrame *frame)
{
	unsigned char *data;
	long size = 0;
	int result = 0;

// Get from audio device
	if(device->sharing)
	{
		device->sharing_lock.lock();
		shared_output_lock[shared_output_number].lock();
		if(!device->done_sharing)
		{
			size = shared_frames[shared_output_number]->get_compressed_size();
			data = shared_frames[shared_output_number]->get_data();

			frame->allocate_compressed_data(size);
			memcpy(frame->get_data(), data, size);
			frame->set_compressed_size(size);
		}
		else
		{
			size = 0;
			data = 0;
		}

		device->sharing_lock.unlock();

		shared_ready[shared_output_number] = 1;
		shared_output_number++;
		if(shared_output_number >= device->in_config->capture_length) shared_output_number = 0;
	}
	else
// Get from libdv directly
	if(grabber)
	{
		result = dv_grab_frame(grabber, &data, &size);

		if(!result)
		{
			frame->allocate_compressed_data(size);
			memcpy(frame->get_data(), data, size);
			frame->set_compressed_size(size);
		}
		dv_unlock_frame(grabber);
	}

	return result;
return 0;
}

int VDevice1394::write_buffer(VFrame *frame)
{
	return 0;
return 0;
}

int VDevice1394::can_copy_from(Asset *asset, int output_w, int output_h)
{
	return 0;
return 0;
}

#endif // HAVE_FIREWIRE
