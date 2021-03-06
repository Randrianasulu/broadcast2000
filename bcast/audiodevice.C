#include <string.h>
#include "audio1394.h"
#include "audioconfig.h"
#include "audiodevice.h"
#include "audioalsa.h"
#include "audioesound.h"
#include "audiooss.h"
#include "dcoffset.h"




AudioLowLevel::AudioLowLevel(AudioDevice *device)
{
	this->device = device;
}

AudioLowLevel::~AudioLowLevel()
{
}






AudioDevice::AudioDevice()
 : Thread()
{
	this->config = new AudioConfig;
	initialize();
}

AudioDevice::~AudioDevice()
{
	delete config;
}

int AudioDevice::initialize()
{
	record_before_play = 0;
	r = w = d = 0;

	for(int i = 0; i < TOTAL_BUFFERS; i++)
	{
		buffer[i] = 0;
		buffer_size[i] = 0;
		play_mutex[i].lock();
		last_buffer[i] = 0;
	}

	input_buffer = 0;
	duplex_init = 0;
	rec_dither = play_dither = 0;
	software_position_info = 0;
	arm_buffer_num = 0;
	is_playing_back = 0;
	is_recording = 0;
	last_buffer_size = total_samples = position_correction = 0;
	last_position = 0;
	dc_offset_thread = new DC_Offset;
	interrupt = 0;
	lowlevel_in = lowlevel_out = lowlevel_duplex = 0;
	vdevice = 0;
	sharing = 0;
	total_samples_read = 0;
	return 0;
return 0;
}

int AudioDevice::create_lowlevel(AudioLowLevel* &lowlevel, int driver)
{
	if(!lowlevel)
	{
		switch(driver)
		{
#ifdef HAVE_OSS
			case AUDIO_OSS:
				lowlevel = new AudioOSS(this);
				break;
#endif

#ifdef HAVE_ESOUND
			case AUDIO_ESOUND:
				lowlevel = new AudioESound(this);
				break;
#endif
			case AUDIO_NAS:
				break;

#ifdef HAVE_ALSA
			case AUDIO_ALSA:
				lowlevel = new AudioALSA(this);
				break;
#endif

#ifdef HAVE_FIREWIRE	
			case AUDIO_1394:
				lowlevel = new Audio1394(this);
				break;
#endif
		}
	}
	return 0;
return 0;
}

int AudioDevice::open_input(AudioConfig *config, int rate, int samples)
{
	r = 1;
	duplex_init = 0;
	*this->config = *config;
	in_samplerate = rate;
	in_samples = samples;
	create_lowlevel(lowlevel_in, config->audio_in_driver);
	lowlevel_in->open_input();
	return 0;
return 0;
}

int AudioDevice::open_output(AudioConfig *config, int rate, int samples)
{
	w = 1;
	duplex_init = 0;
	*this->config = *config;
	out_samplerate = rate;
	out_samples = samples;
	create_lowlevel(lowlevel_out, config->audio_out_driver);
	lowlevel_out->open_output();
	return 0;
return 0;
}

int AudioDevice::open_duplex(AudioConfig *config, int rate, int samples)
{
	d = 1;
	duplex_init = 1;        // notify playback routines to test the duplex lock
	*this->config = *config;
	duplex_samplerate = rate;
	duplex_samples = samples;
	duplex_lock.lock();     // prevent playback until recording starts
	create_lowlevel(lowlevel_duplex, config->audio_duplex_driver);
	lowlevel_duplex->open_duplex();
	return 0;
return 0;
}


int AudioDevice::interrupt_crash()
{
	if(lowlevel_in) return lowlevel_in->interrupt_crash();
	return 0;
return 0;
}


int AudioDevice::close_all()
{
	if(lowlevel_in) lowlevel_in->close_all();
	if(lowlevel_out) lowlevel_out->close_all();
	if(lowlevel_duplex) lowlevel_duplex->close_all();

	reset_output();
	if(input_buffer) 
	{
		delete [] input_buffer; 
		input_buffer = 0; 
	}
	
	is_recording = 0;
	rec_dither = play_dither = 0;
	software_position_info = position_correction = last_buffer_size = 0;
	r = w = d = 0;
	duplex_init = 0;
	vdevice = 0;
	sharing = 0;

	if(lowlevel_in)
	{
		delete lowlevel_in;
		lowlevel_in = 0;
	}
	if(lowlevel_out)
	{
		delete lowlevel_out;
		lowlevel_out = 0;
	}
	if(lowlevel_duplex)
	{
		delete lowlevel_duplex;
		lowlevel_duplex = 0;
	}
	return 0;
return 0;
}

int AudioDevice::set_vdevice(VideoDevice *vdevice)
{
	this->vdevice = vdevice;
	return 0;
return 0;
}


int AudioDevice::get_ichannels()
{
	if(r) return in_channels;
	else if(d) return duplex_channels;
	else return 0;
return 0;
}

int AudioDevice::get_ibits()
{
	if(r) return in_bits;
	else if(d) return duplex_bits;
	return 0;
return 0;
}


int AudioDevice::get_obits()
{
	if(w) return out_bits;
	else if(d) return duplex_bits;
	return 0;
return 0;
}

int AudioDevice::get_ochannels()
{
	if(w) return out_channels;
	else if(d) return duplex_channels;
	return 0;
return 0;
}

AudioLowLevel* AudioDevice::get_lowlevel_out()
{
	if(w) return lowlevel_out;
	else if(d) return lowlevel_duplex;
	return 0;
}

AudioLowLevel* AudioDevice::get_lowlevel_in()
{
	if(r) return lowlevel_in;
	else if(d) return lowlevel_duplex;
	return 0;
}

long AudioDevice::get_orate()
{
	if(w) return out_samplerate;
	else if(d) return duplex_samplerate;
	return 0;
}

int AudioDevice::get_interrupted()
{
	return interrupt;
return 0;
}
