#include <string.h>
#include "audiodevice.h"

int AudioDevice::write_buffer(float **output, long samples, int channels)
{
// find free buffer to fill
	if(interrupt) return 0;
	arm_buffer(arm_buffer_num, output, samples, channels);
	arm_buffer_num++;
	if(arm_buffer_num >= TOTAL_BUFFERS) arm_buffer_num = 0;
return 0;
}

int AudioDevice::set_last_buffer()
{
	arm_mutex[arm_buffer_num].lock();
	last_buffer[arm_buffer_num] = 1;
	play_mutex[arm_buffer_num].unlock();
	arm_buffer_num++;
	if(arm_buffer_num >= TOTAL_BUFFERS) arm_buffer_num = 0;
return 0;
}


// must run before threading once to allocate buffers
// must send maximum size buffer the first time or risk reallocation while threaded
int AudioDevice::arm_buffer(int buffer_num, float **output, long samples, int channels)
{
	int bits;
	long new_size;
	
	long i, j;
	long input_offset;
	long output_offset;
	int output_advance;
	int channel, last_input_channel;
	float sample;
	int int_sample, int_sample2;
	int dither_value;
	int frame;
	int device_channels = get_ochannels();
	char *buffer_num_buffer;
	float *buffer_in_channel;

	if(channels == -1) channels = get_ochannels();
	bits = get_obits();

	frame = device_channels * (bits / 8);
	if(bits == 24) frame = 4;
	
	new_size = frame * samples;

	if(interrupt) return 1;

// wait for buffer to become available for writing
	arm_mutex[buffer_num].lock();
	if(interrupt) return 1;
	
	if(new_size != buffer_size[buffer_num])
	{
		if(new_size > buffer_size[buffer_num])
		{
// do both buffers before threading to prevent kill during allocation
			for(i = 0; i < 2; i++)
			{
				if(buffer_size[i] != 0)
				{
					delete [] buffer[i];
				}
				buffer[i] = new char[new_size];
				buffer_size[i] = new_size;
			}
		}
	}
	buffer_size[buffer_num] = new_size;

	buffer_num_buffer = buffer[buffer_num];
	last_input_channel = channels - 1;
// copy data
// intel byte order only to correspond with bits_to_fmt

	for(channel = 0; channel < device_channels; channel++)
	{
		if(channel >= channels) buffer_in_channel = output[last_input_channel];
		else buffer_in_channel = output[channel];
				
		switch(bits)
		{
			case 8:
				output_advance = device_channels;
				if(play_dither)
				{
					for(output_offset = channel, input_offset = 0; input_offset < samples; output_offset += output_advance, input_offset++)
					{
						sample = buffer_in_channel[input_offset];
						sample *= 0x7fff;
						int_sample = (long)sample;
						dither_value = rand() % 255;
						int_sample -= dither_value;
						int_sample /= 0x100;
						buffer_num_buffer[output_offset] = int_sample;
					}
				}
				else
				{
					for(output_offset = channel, input_offset = 0; input_offset < samples; output_offset += output_advance, input_offset++)
					{
						sample = buffer_in_channel[input_offset];
						sample *= 0x7f;
						int_sample = (long)sample;
						buffer_num_buffer[output_offset] = int_sample;
					}
				}
				break;
			
			case 16:
				output_advance = device_channels * 2 - 1;
				if(play_dither)
				{
					for(output_offset = channel * 2, input_offset = 0; input_offset < samples; output_offset += output_advance, input_offset++)
					{
						sample = buffer_in_channel[input_offset];
						sample *= 0x7fffff;
						int_sample = (long)sample;
						dither_value = rand() % 255;
						int_sample -= dither_value;
						int_sample /= 0x100;
						buffer_num_buffer[output_offset] = int_sample;
					}
				}
				else
				{
					for(output_offset = channel * 2, input_offset = 0; input_offset < samples; output_offset += output_advance, input_offset++)
					{
						sample = buffer_in_channel[input_offset];
						sample *= 0x7fff;
						int_sample = (long)sample;
						buffer_num_buffer[output_offset++] = (int_sample & 0xFF);
						int_sample &= 0xff00;
						buffer_num_buffer[output_offset] = int_sample >> 8;
					}
				}
				break;
			
			case 24:
				output_advance = device_channels * 4 - 2;
				for(output_offset = channel * 4, input_offset = 0; 
					input_offset < samples; 
					output_offset += output_advance, input_offset += 2)
				{
					sample = buffer_in_channel[input_offset];
					sample *= 0x7fffff;
					int_sample2 = int_sample = (long)sample;
					buffer_num_buffer[output_offset++] = (int_sample & 0xFF);
					int_sample &= 0xff00;
					buffer_num_buffer[output_offset++] = int_sample >> 8;
					int_sample2 &= 0xff0000;
					buffer_num_buffer[output_offset] = int_sample2 >> 16;
				}
				break;
		}
	}
	
// make buffer available for playback
	play_mutex[buffer_num].unlock();
return 0;
}

int AudioDevice::reset_output()
{
	for(int i = 0; i < TOTAL_BUFFERS; i++)
	{
		if(buffer_size[i]) { delete [] buffer[i]; }
		buffer[i] = 0;
		buffer_size[i] = 0;
		arm_mutex[i].unlock();
		play_mutex[i].reset(); 
		play_mutex[i].lock(); 
		last_buffer[i] = 0;
	}

// unlock mutexes
//complete.unlock();
	is_playing_back = 0;
	software_position_info = position_correction = last_buffer_size = 0;
	total_samples = 0;
	play_dither == 0;
	arm_buffer_num = 0;
	last_position = 0;
	interrupt = 0;
return 0;
}


int AudioDevice::set_play_dither(int status)
{
	play_dither = status;
return 0;
}

int AudioDevice::set_software_positioning(int status)
{
	software_position_info = status;
return 0;
}

int AudioDevice::start_playback()
{
// arm buffer before doing this
	Thread::synchronous = 1;
	is_playing_back = 1;
	interrupt = 0;
// zero timers
	buffer_timer.update();       
	last_position = 0;
	startup_lock.lock();

	start();                  // synchronize threads by starting playback here and blocking
return 0;
}

int AudioDevice::interrupt_playback()
{
	interrupt = 1;

	if(is_playing_back)
	{
// end thread forcefully
		is_playing_back = 0;
		get_lowlevel_out()->interrupt_playback();
		Thread::end();
		wait_for_completion();
	}

// unlock everything
	for(int i = 0; i < TOTAL_BUFFERS; i++)
	{
		play_mutex[i].reset();
//		play_mutex[i].unlock();  // Causes a crash when run() is waiting on it
		arm_mutex[i].unlock();
	}

	return 0;
return 0;
}

int AudioDevice::wait_for_startup()
{
	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

int AudioDevice::wait_for_completion()
{
	join();
	return 0;
return 0;
}



long AudioDevice::current_position()
{
// try to get OSS position
	long hardware_result = 0, software_result = 0, frame;

	if(w)
	{
		frame = get_obits() / 8;
		if(get_obits() == 24) frame = 4;

// get hardware position
		if(!software_position_info)
		{
			hardware_result = get_lowlevel_out()->device_position();
		}

// get software position
		if(hardware_result < 0 || software_position_info)
		{
			timer_lock.lock();
			software_result = total_samples - last_buffer_size - 
				device_buffer / frame / get_ochannels();
			software_result += buffer_timer.get_scaled_difference(get_orate());
			timer_lock.unlock();

			if(software_result < last_position) 
			software_result = last_position;
			else
			last_position = software_result;
		}
	}
	else
	if(r)
	{
		return total_samples_read;
	}

	if(hardware_result < 0 || software_position_info) 
		return software_result;
	else
		return hardware_result;
}

void AudioDevice::run()
{
	thread_buffer_num = 0;

	startup_lock.unlock();
	buffer_timer.update();

	while(is_playing_back && !interrupt && !last_buffer[thread_buffer_num])
	{
// wait for buffer to become available
		play_mutex[thread_buffer_num].lock();

		if(is_playing_back && !last_buffer[thread_buffer_num])
		{
			if(duplex_init)
			{
				if(record_before_play)
				{
// block until recording starts
					duplex_lock.lock();
				}
				else
				{
// allow recording to start
					duplex_lock.unlock();
				}
				duplex_init = 0;
			}

// get size for position information
			timer_lock.lock();
			last_buffer_size = buffer_size[thread_buffer_num] / (get_obits() / 8) / get_ochannels();
			total_samples += last_buffer_size;
			buffer_timer.update();

			timer_lock.unlock();

// write buffer
			thread_result = get_lowlevel_out()->write_buffer(buffer[thread_buffer_num], buffer_size[thread_buffer_num]);

// allow writing to the buffer
			arm_mutex[thread_buffer_num].unlock();

// inform user if the buffer write failed
			if(thread_result < 0)
			{
				perror("AudioDevice::write_buffer");
				sleep(1);
			}

			thread_buffer_num++;
			if(thread_buffer_num >= TOTAL_BUFFERS) thread_buffer_num = 0;
		}

// test for last buffer
		if(!interrupt && last_buffer[thread_buffer_num])
		{
// no more buffers
			is_playing_back = 0;
// flush the audio device
			get_lowlevel_out()->flush_device();
		}
	}
}
