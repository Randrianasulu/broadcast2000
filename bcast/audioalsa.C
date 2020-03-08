#include <string.h>
#include "audiodevice.h"
#include "audioalsa.h"

#ifdef HAVE_ALSA

AudioALSA::AudioALSA(AudioDevice *device) : AudioLowLevel(device)
{
}

AudioALSA::~AudioALSA()
{
}

int AudioALSA::create_format(snd_pcm_format_t *format, int bits, int channels, int rate)
{
	format->format = snd_pcm_build_linear_format(bits, 0, 0);
	format->rate = rate;
	format->interleave = 1;
	format->voices = channels;
return 0;
}

int AudioALSA::open_output()
{
	int err;
	if(err = snd_pcm_open(&dsp_out, 1, 0, SND_PCM_OPEN_PLAYBACK) < 0)
	{
		fprintf(stderr, "AudioALSA::open_output: %s\n", snd_strerror(err));
		return 1;
	}

	snd_pcm_channel_params_t params;
	create_format(&params.format, device->obits, device->ochannels, device->orate);
	snd_pcm_channel_params(dsp_out, &params);
	snd_pcm_playback_prepare(dsp_out);
	snd_pcm_playback_go(dsp_out);

	device->device_buffer = 0;
	return 0;
return 0;
}

int AudioALSA::open_input()
{
	int err;
	if(err = snd_pcm_open(&dsp_out, 0, 0, SND_PCM_OPEN_CAPTURE) < 0)
	{
		fprintf(stderr, "AudioALSA::open_input: %s\n", snd_strerror(err));
		return 1;
	}

	snd_pcm_channel_params_t params;
	create_format(&params.format, device->ibits, device->ichannels, device->irate);
	snd_pcm_channel_params(dsp_in, &params);
	snd_pcm_capture_prepare(dsp_in);
	snd_pcm_capture_go(dsp_in);
	return 0;
return 0;
}

int AudioALSA::open_duplex()
{
	int err;
	if(err = snd_pcm_open(&dsp_duplex, 0, 0, SND_PCM_OPEN_DUPLEX) < 0)
	{
		fprintf(stderr, "AudioALSA::open_duplex: %s\n", snd_strerror(err));
		return 1;
	}

	snd_pcm_channel_params_t params;
	create_format(&params.format, device->dbits, device->dchannels, device->drate);
	snd_pcm_channel_params(dsp_duplex, &params);
	snd_pcm_playback_prepare(dsp_duplex);
	snd_pcm_playback_go(dsp_duplex);
	snd_pcm_capture_prepare(dsp_duplex);
	snd_pcm_capture_go(dsp_duplex);

	device->device_buffer = 0;
	return 0;
return 0;
}

int AudioALSA::close_all()
{
	if(device->r) 
	{ 
		snd_pcm_close(dsp_in);
	}

	if(device->w) 
	{ 
		snd_pcm_close(dsp_out);
	}

	if(device->d) 
	{ 
		snd_pcm_close(dsp_duplex);
	}
return 0;
}

// Undocumented
long AudioALSA::device_position()
{
	return -1;
}

int AudioALSA::read_buffer(char *buffer, long size)
{
	snd_pcm_read(get_input(), buffer, size);
	return 0;
return 0;
}

int AudioALSA::write_buffer(char *buffer, long size)
{
	snd_pcm_write(get_input(), buffer, size);
	return 0;
return 0;
}

int AudioALSA::flush_device()
{
	snd_pcm_playback_flush(get_output());
	return 0;
return 0;
}

int AudioALSA::interrupt_playback()
{
	snd_pcm_playback_drain(get_output());
	return 0;
return 0;
}


snd_pcm_t* AudioALSA::get_output()
{
	if(device->w) return dsp_out;
	else if(device->d) return dsp_duplex;
	return 0;
}

snd_pcm_t* AudioALSA::get_input()
{
	if(device->r) return dsp_in;
	else if(device->d) return dsp_duplex;
	return 0;
}

#endif
