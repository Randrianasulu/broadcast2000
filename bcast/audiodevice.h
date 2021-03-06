#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "audioconfig.inc"
#include "audiodevice.inc"
#include "audio1394.inc"
#include "audioesound.inc"
#include "audiooss.inc"
#include "binary.h"
#include "dcoffset.inc"
#include "maxchannels.h"
#include "mutex.h"
#include "recordgui.inc"
#include "thread.h"
#include "timer.h"
#include "vdevice1394.inc"
#include "videodevice.inc"

class AudioLowLevel
{
public:
	AudioLowLevel(AudioDevice *device);
	virtual ~AudioLowLevel();

	virtual int open_input() { return 1; };
	virtual int open_output() { return 1; };
	virtual int open_duplex() { return 1; };
	virtual int close_all() { return 1; };
	virtual int interrupt_crash() { return 0; };
	virtual long device_position() { return -1; };
	virtual int write_buffer(char *buffer, long size) { return 1; };
	virtual int read_buffer(char *buffer, long size) { return 1; };
	virtual int flush_device() { return 1; };
	virtual int interrupt_playback() { return 1; };
	
	AudioDevice *device;
};


class AudioDevice : public Thread
{
public:
	AudioDevice();
	~AudioDevice();

	friend AudioOSS;
	friend AudioESound;
	friend Audio1394;
	friend VDevice1394;

	int open_input(AudioConfig *config, int rate, int samples);
	int open_output(AudioConfig *config, int rate, int samples);
	int open_duplex(AudioConfig *config, int rate, int samples);
	int close_all();
	int reset_output();
	int restart();

// Specify a video device to pass data to if the same device handles video
	int set_vdevice(VideoDevice *vdevice);

// ================================ recording

	int read_buffer(float **input, long samples, int channels, int *over, float *max, long input_offset = 0);  // read from the record device
	int set_record_dither(int value);

	int stop_recording();
// If a firewire device crashed
	int interrupt_crash();

// ================================== dc offset

// get and set offset
	int get_dc_offset(long *output, RecordGUIDCOffsetText **dc_offset_text);
// set new offset
	int set_dc_offset(long dc_offset, int channel);
// writes to whichever buffer is free or blocks until one becomes free
	int write_buffer(float **output, long samples, int channels = -1); 

// play back buffers
	void run();           

// After the last buffer is written call this to terminate.
// A separate buffer for termination is required since the audio device can be
// finished without writing a single byte
	int set_last_buffer();         

	int wait_for_startup();
// wait for the playback thread to clean up
	int wait_for_completion();

// start the thread processing buffers
	int start_playback();
// interrupt the playback thread
	int interrupt_playback();
	int set_play_dither(int status);
// set software positioning on or off
	int set_software_positioning(int status = 1);
// total samples played
	long current_position();
// If interrupted
	int get_interrupted();


private:
	int initialize();
// Create a lowlevel driver out of the driver ID
	int create_lowlevel(AudioLowLevel* &lowlevel, int driver);
	int arm_buffer(int buffer, float **output, long samples, int channels);
	AudioLowLevel* get_lowlevel_out();
	int get_obits();
	int get_ochannels();
	int get_ibits();
	int get_ichannels();
	long get_orate();
	AudioLowLevel* get_lowlevel_in();

	DC_Offset *dc_offset_thread;
// Override configured parameters depending on the driver
	int in_samplerate, in_bits, in_channels, in_samples;
	int out_samplerate, out_bits, out_channels, out_samples;
	int duplex_samplerate, duplex_bits, duplex_channels, duplex_samples;

// Access mode
	int r, w, d;
// Samples per buffer
	long osamples, isamples, dsamples;
// Video device to pass data to if the same device handles video
	VideoDevice *vdevice;
// OSS < 3.9   --> playback before recording
// OSS >= 3.9  --> doesn't matter
// Got better synchronization by starting playback first
	int record_before_play;
	Mutex duplex_lock, startup_lock;
// notify playback routines to test the duplex lock
	int duplex_init;        
// bits in output file
	int rec_dither;         
// 1 or 0
	int play_dither;        
	int sharing;

	long buffer_size[TOTAL_BUFFERS];
	int last_buffer[TOTAL_BUFFERS];    // not written to device
// formatted buffers for output
	char *buffer[TOTAL_BUFFERS], *input_buffer;
	Mutex play_mutex[TOTAL_BUFFERS], arm_mutex[TOTAL_BUFFERS];
	Mutex timer_lock;
	int arm_buffer_num;

// for position information
	long total_samples, last_buffer_size, position_correction;
	long device_buffer;
	long last_position;  // prevent the counter from going backwards
	Timer buffer_timer, global_timer;
// Current operation
	int is_playing_back, is_recording, global_timer_started, software_position_info;
	int interrupt;

// Multiple data paths can be opened simultaneously by RecordEngine
	AudioLowLevel *lowlevel_in, *lowlevel_out, *lowlevel_duplex;
	AudioConfig *config;

private:
	int thread_buffer_num, thread_result;
	long total_samples_read;
};



#endif
