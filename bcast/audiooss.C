#include <string.h>
#include "audioconfig.h"
#include "audiodevice.h"
#include "audiooss.h"

#ifdef HAVE_OSS

// These are only available in commercial OSS

#ifndef AFMT_S32_LE
#define AFMT_S32_LE 	 0x00001000
#define AFMT_S32_BE 	 0x00002000
#endif

AudioOSS::AudioOSS(AudioDevice *device) : AudioLowLevel(device)
{
	dsp_in = dsp_out = dsp_duplex = 0;
}

AudioOSS::~AudioOSS()
{
}

int AudioOSS::open_input()
{
	device->in_channels = device->config->oss_in_channels;
	device->in_bits = device->config->oss_in_bits;
// 24 bits not available in OSS
	if(device->in_bits == 24) device->in_bits = 32;

	dsp_in = open(device->config->oss_in_device, O_RDONLY/* | O_NDELAY*/);
	if(dsp_in < 0) perror("AudioOSS::open_input");

	int format = get_fmt(device->in_bits);
	int buffer_info = sizetofrag(device->in_samples, device->in_channels, device->in_bits);

	set_cloexec_flag(dsp_in, 1);

	if(ioctl(dsp_in, SNDCTL_DSP_SETFRAGMENT, &buffer_info)) printf("SNDCTL_DSP_SETFRAGMENT failed.\n");
	if(ioctl(dsp_in, SNDCTL_DSP_SETFMT, &format) < 0) printf("SNDCTL_DSP_SETFMT failed\n");
	if(ioctl(dsp_in, SNDCTL_DSP_CHANNELS, &device->in_channels) < 0) printf("SNDCTL_DSP_CHANNELS failed\n");
	if(ioctl(dsp_in, SNDCTL_DSP_SPEED, &device->in_samplerate) < 0) printf("SNDCTL_DSP_SPEED failed\n");
	return 0;
return 0;
}

int AudioOSS::open_output()
{
	device->out_channels = device->config->oss_out_channels;
	device->out_bits = device->config->oss_out_bits;
	if(device->out_bits == 24) device->out_bits = 32;

	dsp_out = open(device->config->oss_out_device, O_WRONLY /*| O_NDELAY*/);
	if(dsp_out < 0) perror("AudioOSS::open_output");

	int format = get_fmt(device->out_bits);
	int buffer_info = sizetofrag(device->out_samples, device->out_channels, device->out_bits);
	audio_buf_info playinfo;

	set_cloexec_flag(dsp_out, 1);

	if(ioctl(dsp_out, SNDCTL_DSP_SETFRAGMENT, &buffer_info)) printf("SNDCTL_DSP_SETFRAGMENT failed.\n");
	if(ioctl(dsp_out, SNDCTL_DSP_SETFMT, &format) < 0) printf("SNDCTL_DSP_SETFMT failed\n");
	if(ioctl(dsp_out, SNDCTL_DSP_CHANNELS, &device->out_channels) < 0) printf("SNDCTL_DSP_CHANNELS failed\n");
	if(ioctl(dsp_out, SNDCTL_DSP_SPEED, &device->out_samplerate) < 0) printf("SNDCTL_DSP_SPEED failed\n");
	ioctl(dsp_out, SNDCTL_DSP_GETOSPACE, &playinfo);
	device->device_buffer = playinfo.bytes;
	return 0;
return 0;
}

int AudioOSS::open_duplex()
{
	device->duplex_channels = device->config->oss_duplex_channels;
	device->duplex_bits = device->config->oss_duplex_bits;
	if(device->duplex_bits == 24) device->duplex_bits = 32;

	dsp_duplex = open(device->config->oss_duplex_device, O_RDWR/* | O_NDELAY*/);
	if(dsp_duplex < 0) perror("AudioOSS::open_duplex");

	int format = get_fmt(device->duplex_bits);
	int buffer_info = sizetofrag(device->duplex_samples, device->duplex_channels, device->duplex_bits);
	audio_buf_info playinfo;

	set_cloexec_flag(dsp_duplex, 1);

	if(ioctl(dsp_duplex, SNDCTL_DSP_SETFRAGMENT, &buffer_info)) printf("SNDCTL_DSP_SETFRAGMENT failed.\n");
	if(ioctl(dsp_duplex, SNDCTL_DSP_SETDUPLEX, 1) == -1) printf("SNDCTL_DSP_SETDUPLEX failed\n");
	if(ioctl(dsp_duplex, SNDCTL_DSP_SETFMT, &format) < 0) printf("SNDCTL_DSP_SETFMT failed\n");
	if(ioctl(dsp_duplex, SNDCTL_DSP_CHANNELS, &device->duplex_channels) < 0) printf("SNDCTL_DSP_CHANNELS failed\n");
	if(ioctl(dsp_duplex, SNDCTL_DSP_SPEED, &device->duplex_samplerate) < 0) printf("SNDCTL_DSP_SPEED failed\n");
	ioctl(dsp_duplex, SNDCTL_DSP_GETOSPACE, &playinfo);
	device->device_buffer = playinfo.bytes;
	return 0;
return 0;
}

int AudioOSS::sizetofrag(int samples, int channels, int bits)
{
	int testfrag = 2, fragsize = 1;
	samples *= channels * bits / 8;
	while(testfrag < samples)
	{
		fragsize++;
		testfrag *= 2;
	}
	return (4 << 16) | fragsize;
return 0;
}

int AudioOSS::get_fmt(int bits)
{
	switch(bits)
	{
		case 32: return AFMT_S32_LE; break;
		case 16: return AFMT_S16_LE; break;
		case 8:  return AFMT_S8;  break;
	}
	return AFMT_S16_LE;
return 0;
}


int AudioOSS::close_all()
{
	if(device->r) 
	{ 
		ioctl(dsp_in, SNDCTL_DSP_RESET, 0);         
		close(dsp_in);      
	}

	if(device->w) 
	{ 
		ioctl(dsp_out, SNDCTL_DSP_RESET, 0);        
		close(dsp_out);     
	}

	if(device->d) 
	{ 
		ioctl(dsp_duplex, SNDCTL_DSP_RESET, 0);     
		close(dsp_duplex);  
	}
return 0;
}

int AudioOSS::set_cloexec_flag(int desc, int value)
{
	int oldflags = fcntl (desc, F_GETFD, 0);
	if (oldflags < 0) return oldflags;
	if(value != 0) 
		oldflags |= FD_CLOEXEC;
	else
		oldflags &= ~FD_CLOEXEC;
	return fcntl(desc, F_SETFD, oldflags);
return 0;
}

long AudioOSS::device_position()
{
	count_info info;
	if(!ioctl(get_output(), SNDCTL_DSP_GETOPTR, &info))
	{
		return info.bytes / (device->get_obits() / 8) / device->get_ochannels();
	}
return 0;
}

int AudioOSS::interrupt_playback()
{
	return 0;
}

int AudioOSS::read_buffer(char *buffer, long size)
{
	return read(get_input(), buffer, size);
}

int AudioOSS::write_buffer(char *buffer, long size)
{
	return write(get_output(), buffer, size);
}

int AudioOSS::flush_device()
{
	ioctl(get_output(), SNDCTL_DSP_SYNC, 0);
return 0;
}

int AudioOSS::get_output()
{
	if(device->w) return dsp_out;
	else if(device->d) return dsp_duplex;
	return 0;
return 0;
}

int AudioOSS::get_input()
{
	if(device->r) return dsp_in;
	else if(device->d) return dsp_duplex;
	return 0;
return 0;
}

#endif // HAVE_OSS
