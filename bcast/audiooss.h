#ifndef AUDIOOSS_H
#define AUDIOOSS_H

#include "audiodevice.inc"

#ifdef HAVE_OSS
#include <sys/soundcard.h>

class AudioOSS : public AudioLowLevel
{
public:
	AudioOSS(AudioDevice *device);
	~AudioOSS();
	
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
	int get_fmt(int bits);
	int sizetofrag(int samples, int channels, int bits);
	int set_cloexec_flag(int desc, int value);
	int get_output();
	int get_input();
	int dsp_in, dsp_out, dsp_duplex;
};

#endif

#endif
