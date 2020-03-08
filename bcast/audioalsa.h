#ifndef AUDIOALSA_H
#define AUDIOALSA_H

#include "audiodevice.inc"

#ifdef HAVE_ALSA
#include <sys/asoundlib.h>

class AudioALSA : public AudioLowLevel
{
public:
	AudioALSA(AudioDevice *device);
	~AudioALSA();
	
	int open_input();
	int open_output();
	int open_duplex();
	int write_buffer(char *buffer, long size);
	int read_buffer(char *buffer, long size);
	int close_all();
	long device_position();
	int flush_device();
	int interrupt_playback();

private:
	int create_format(snd_pcm_format_t *format, int bits, int channels, int rate);
	snd_pcm_t* get_output();
	snd_pcm_t* get_input();
	snd_pcm_t *dsp_in, *dsp_out, *dsp_duplex;
};

#endif
#endif
