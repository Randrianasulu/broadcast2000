// V4L2 is incompatible with large file support
#undef _FILE_OFFSET_BITS
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE

#include <string.h>
#include "assets.h"
#include "bccapture.h"
#include "channel.h"
#include "chantables.h"
#include "mainwindow.h"
#include "playbackengine.h"
#include "recvideowindow.h"
#include "vdevice1394.h"
#include "vdevicelml.h"
#include "vdevicex11.h"
#include "videoconfig.h"
#include "videodevice.h"
#include "videowindow.h"
#include "videowindowgui.h"
#include "vframe.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>

KeepaliveThread::KeepaliveThread(VideoDevice *device) : Thread()
{
	still_alive = 1;
	failed = 0;
	interrupted = 0;
	Thread::synchronous = 1;
	this->device = device;
	capturing = 0;
}

KeepaliveThread::~KeepaliveThread()
{
}

int KeepaliveThread::start_keepalive()
{
	startup_lock.lock();
	start();
	startup_lock.lock();
	startup_lock.unlock();
return 0;
}

void KeepaliveThread::run()
{
	startup_lock.unlock();
	while(!interrupted)
	{
		still_alive = 0;
// Give the capture a moment
// Should fix the delay in case users want slower frame rates.
		timer.delay(KEEPALIVE_DELAY * 1000);

// See if a capture happened
		if(still_alive == 0 && capturing)
		{
			printf("KeepaliveThread::run: device crashed\n");
			failed++;
		}
		else
			failed = 0;
	}
}

int KeepaliveThread::reset_keepalive()
{
	still_alive = 1;
return 0;
}

int KeepaliveThread::get_failed()
{
	if(failed) return 1; else return 0;
return 0;
}

int KeepaliveThread::stop()
{
	interrupted = 1;

// Force an immediate exit even if capture_frame worked.
	Thread::end();
	Thread::join();
return 0;
}







VideoDevice::VideoDevice()
{
	mwindow = 0;
	in_config = new VideoConfig;
	out_config = new VideoConfig;
	initialize();
}

VideoDevice::VideoDevice(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	in_config = new VideoConfig;
	out_config = new VideoConfig;
	initialize();
}

VideoDevice::~VideoDevice()
{
	for(int i = 0; i < input_sources.total; i++)
		delete input_sources.values[i];
	input_sources.remove_all();
	delete in_config;
	delete out_config;
}

int VideoDevice::initialize()
{
	sharing = 0;
	done_sharing = 0;
	sharing_lock.reset();
	orate = irate = 0;
	out_w = out_h = 0;
	in_w = in_h = 0;
	r = w = 0;
	is_playing_back = is_recording = 0;
	capture_buffer = 0;
	shared_memory = 0;
	input_x = 0;
	input_y = 0;
	input_z = 1;
	frame_resized = 0;
	capture_bitmap = 0;
	capturing = 0;
	keepalive = 0;
	v4l2_buffer_list = 0;
	swap_bytes = 0;
	input_base = 0;
	output_base = 0;
	slippery = 0;
	output_format = 0;
	interrupt = 0;
	adevice = 0;
return 0;
}

int VideoDevice::open_input(VideoConfig *config, 
	int frame_w, 
	int frame_h, 
	int input_x, 
	int input_y, 
	float input_z,
	float frame_rate)
{
	int i;
	struct v4l2_capability cap2;    // Video4linux 2 test
	struct video_capability cap1;   // Video4linux 1 test

	*this->in_config = *config;

	r = 1;
	this->in_w = frame_w;
	this->in_h = frame_h;
	this->input_z = -1;   // Force initialization.
	capture_buffer = 0;
	capture_frame_number = 0;
	read_frame_number = 0;
	capturing = 1;
	this->frame_rate = frame_rate;

// Open the driver using the updated version
	if(in_config->video_in_driver == VIDEO4LINUX || in_config->video_in_driver == VIDEO4LINUX2)
	{
// Get the proper video4linux API by testing calls only available in the newest driver
		if((input_fd = open(in_config->v4l_in_device, O_RDWR)) < 0)
			perror("VideoDevice::open_input");
		else
		{
			set_cloexec_flag(input_fd, 1);
			if(ioctl(input_fd, VIDIOC_QUERYCAP, &cap2) != -1)
			{
				in_config->video_in_driver = VIDEO4LINUX2;
			}
			else
			if(ioctl(input_fd, VIDIOCGCAP, &cap1) != -1)
			{
				in_config->video_in_driver = VIDEO4LINUX;
			}
			close(input_fd);
		}

// Open the device for real
		input_fd = open(in_config->v4l_in_device, O_RDWR);
		set_cloexec_flag(input_fd, 1);

// Enable the audio which is obviously handled by the video device.
// Also resets the input source.
		set_mute(0);

		if(input_fd < 0)
			perror("VideoDevice::open_input");
		else
		{
			frame_resized = 1;
			set_translation(input_x, input_y, input_z);
			update_translation();
			if(in_config->video_in_driver == VIDEO4LINUX ||
				in_config->video_in_driver == VIDEO4LINUX2) v4l1_get_inputs();
		}
		keepalive = new KeepaliveThread(this);
		keepalive->start_keepalive();
	}
	else
	if(in_config->video_in_driver == SCREENCAPTURE)
	{
		this->input_x = input_x;
		this->input_y = input_y;
		this->capture_bitmap = new BC_Capture(in_w, in_h, in_config->screencapture_display);
	}
	else
	if(in_config->video_in_driver == CAPTURE_LML)
	{
		keepalive = new KeepaliveThread(this);
		keepalive->start_keepalive();
		input_base = new VDeviceLML(this);
		return input_base->open_input();
	}
#ifdef HAVE_FIREWIRE
	else
	if(in_config->video_in_driver == CAPTURE_FIREWIRE)
	{
		input_base = new VDevice1394(this);
		return input_base->open_input();
	}
#endif

	return 0;
return 0;
}

int VideoDevice::close_all()
{
	int i;
	if(w)
	{
		if(output_base)
		{
			output_base->close_all();
			delete output_base;
		}
	}

	if(r && capturing)
	{
		capturing = 0;
		if(in_config->video_in_driver == VIDEO4LINUX)
		{
			close_v4l();
		}
		else
		if(in_config->video_in_driver == VIDEO4LINUX2)
		{
			close_v4l2();
		}
		else
		if(input_base)
		{
			input_base->close_all();
			delete input_base;
		}

		if(keepalive)
		{
			keepalive->stop();
			delete keepalive;
		}

		if(capture_bitmap) delete capture_bitmap;
	}

	for(int i = 0; i < input_sources.total; i++)
		delete input_sources.values[i];
	input_sources.remove_all();

	initialize();
return 0;
}

int VideoDevice::set_adevice(AudioDevice *adevice)
{
	this->adevice = adevice;
	return 0;
return 0;
}


int VideoDevice::stop_sharing()
{
	if(sharing)
	{
		sharing_lock.lock();
		done_sharing = 1;
		if(input_base) input_base->stop_sharing();
	}
	return 0;
return 0;
}


int VideoDevice::get_shared_data(unsigned char *data, long size)
{
	if(input_base)
	{
		input_base->get_shared_data(data, size);
	}
	return 0;
return 0;
}

int VideoDevice::set_cloexec_flag(int desc, int value)
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

int VideoDevice::v4l1_get_inputs()
{
	struct video_channel channel_struct;
	int i = 0, done = 0;
	char *new_source;

	while(!done)
	{
		channel_struct.channel = i;
		if(ioctl(input_fd, VIDIOCGCHAN, &channel_struct) < 0)
		{
//			perror("VideoDevice::get_inputs");
			done = 1;
		}
		else
		{
			input_sources.append(new_source = new char[strlen(channel_struct.name) + 1]);
			strcpy(new_source, channel_struct.name);
		}
		i++;
	}
return 0;
}

ArrayList<char *>* VideoDevice::get_inputs()
{
	return &input_sources;
}

int VideoDevice::get_failed()
{
	if(keepalive)
		return keepalive->get_failed();
	else
		return 0;
return 0;
}

int VideoDevice::interrupt_crash()
{
	if(input_base) return input_base->interrupt_crash();
	return 0;
return 0;
}

int VideoDevice::set_translation(int input_x, int input_y, float input_z)
{
	new_input_x = input_x;
	new_input_y = input_y;
	new_input_z = input_z;
	if(this->input_x != new_input_x || 
		this->input_y != new_input_y || 
		this->input_z != new_input_z)
	{
		frame_resized = 1;
	}
return 0;
}

int VideoDevice::set_mute(int muted)
{
// Open audio, which obviously is controlled by the video driver.
// And apparently resets the input source.
	if(in_config->video_in_driver == VIDEO4LINUX)
	{
		v4l1_set_mute(muted);
	}
return 0;
}

int VideoDevice::v4l1_set_mute(int muted)
{
	struct video_audio audio;
	audio.audio = 1;

	if(-1 == ioctl(input_fd, VIDIOCGAUDIO, &audio))
	    perror("VideoDevice::ioctl VIDIOCGAUDIO");

	audio.volume = 65535;
	audio.bass = 65535;
	audio.treble = 65535;
	if(muted)
		audio.flags |= VIDEO_AUDIO_MUTE | VIDEO_AUDIO_VOLUME;
	else
		audio.flags &= ~VIDEO_AUDIO_MUTE;

    if (-1 == ioctl(input_fd, VIDIOCSAUDIO, &audio))
		perror("VideoDevice::ioctl VIDIOCSAUDIO");
return 0;
}

int VideoDevice::set_field_order(int odd_field_first)
{
	this->odd_field_first = !odd_field_first;
	return 0;
return 0;
}

int VideoDevice::set_channel(Channel *channel)
{
	if(in_config->video_in_driver == VIDEO4LINUX ||
		in_config->video_in_driver == VIDEO4LINUX2) v4l1_set_channel(channel);
return 0;
}

int VideoDevice::v4l1_set_channel(Channel *channel)
{
//return 0;
	struct video_channel channel_struct;
	struct video_tuner tuner_struct;
	unsigned long new_freq;

// Mute changed the input to TV
//	set_mute(1);

// Read norm/input defaults
	channel_struct.channel = channel->input;
	if(ioctl(input_fd, VIDIOCGCHAN, &channel_struct) < 0)
		perror("VideoDevice::v4l1_set_channel VIDIOCGCHAN");

// Set norm/input
	channel_struct.channel = channel->input;
	channel_struct.norm = v4l1_get_norm(channel->norm);
	if(ioctl(input_fd, VIDIOCSCHAN, &channel_struct) < 0)
		perror("VideoDevice::v4l1_set_channel VIDIOCSCHAN");

	if(channel_struct.flags & VIDEO_VC_TUNER)
	{
// Read tuner defaults
		tuner_struct.tuner = channel->input;
		if(ioctl(input_fd, VIDIOCGTUNER, &tuner_struct) < 0)
			perror("VideoDevice::v4l1_set_channel VIDIOCGTUNER");

// Set tuner
		tuner_struct.mode = v4l1_get_norm(channel->norm);
		if(ioctl(input_fd, VIDIOCSTUNER, &tuner_struct) < 0)
			perror("VideoDevice::v4l1_set_channel VIDIOCSTUNER");

		new_freq = chanlists[channel->freqtable].list[channel->entry].freq;
		new_freq = (int)(new_freq * 0.016);
		new_freq += channel->fine_tune;

		if(ioctl(input_fd, VIDIOCSFREQ, &new_freq) < 0)
			perror("VideoDevice::v4l1_set_channel VIDIOCSFREQ");
	}
//	set_mute(0);
return 0;
}

int VideoDevice::v4l1_get_norm(int norm)
{
	switch(norm)
	{
		case NTSC:         return VIDEO_MODE_NTSC;         break;
		case PAL:          return VIDEO_MODE_PAL;          break;
		case SECAM:        return VIDEO_MODE_SECAM;        break;
	}
return 0;
}

int VideoDevice::set_picture(int brightness, 
	int hue, 
	int color, 
	int contrast, 
	int whiteness)
{
	if(in_config->video_in_driver == VIDEO4LINUX ||
		in_config->video_in_driver == VIDEO4LINUX2) 
		v4l1_set_picture(brightness, 
			hue, 
			color, 
			contrast, 
			whiteness);
	return 0;
return 0;
}


int VideoDevice::v4l1_set_picture(int brightness, 
	int hue, 
	int color, 
	int contrast, 
	int whiteness)
{
	brightness = (int)((float)brightness / 100 * 32767 + 32768);
	hue = (int)((float)hue / 100 * 32767 + 32768);
	color = (int)((float)color / 100 * 32767 + 32768);
	contrast = (int)((float)contrast / 100 * 32767 + 32768);
	whiteness = (int)((float)whiteness / 100 * 32767 + 32768);
	if(ioctl(input_fd, VIDIOCGPICT, &picture_params) < 0)
		perror("VideoDevice::init_video4linux VIDIOCGPICT");
	picture_params.brightness = brightness;
	picture_params.hue = hue;
	picture_params.colour = color;
	picture_params.contrast = contrast;
	picture_params.whiteness = whiteness;
// Bogus.  Values are only set in the capture routine.
	picture_params.depth = 3;
	picture_params.palette = VIDEO_PALETTE_YUV422P;
	if(ioctl(input_fd, VIDIOCSPICT, &picture_params) < 0)
		perror("VideoDevice::init_video4linux VIDIOCSPICT");
	if(ioctl(input_fd, VIDIOCGPICT, &picture_params) < 0)
		perror("VideoDevice::init_video4linux VIDIOCGPICT");
return 0;
}

int VideoDevice::update_translation()
{
	float frame_in_capture_x1f, frame_in_capture_x2f, frame_in_capture_y1f, frame_in_capture_y2f;
	float capture_in_frame_x1f, capture_in_frame_x2f, capture_in_frame_y1f, capture_in_frame_y2f;
	int z_changed = 0;

	if(frame_resized)
	{
		input_x = new_input_x;
		input_y = new_input_y;

		if(in_config->video_in_driver == VIDEO4LINUX || in_config->video_in_driver == VIDEO4LINUX2)
		{
			if(input_z != new_input_z)
			{
				input_z = new_input_z;
				z_changed = 1;

				capture_w = (int)((float)in_w * input_z + 0.5);
				capture_h = (int)((float)in_h * input_z + 0.5);

// Need to align to multiple of 4
				capture_w &= ~3;
				capture_h &= ~3;
				
				if(in_config->video_in_driver == VIDEO4LINUX)
					init_video4linux();
				else
					init_video4linux2();
			}

			frame_in_capture_x1f = (float)input_x * input_z + capture_w / 2 - in_w / 2;
			frame_in_capture_x2f = (float)input_x * input_z  + capture_w / 2 + in_w / 2;
			frame_in_capture_y1f = (float)input_y * input_z  + capture_h / 2 - in_h / 2;
			frame_in_capture_y2f = (float)input_y * input_z  + capture_h / 2 + in_h / 2;

			capture_in_frame_x1f = 0;
			capture_in_frame_y1f = 0;
			capture_in_frame_x2f = in_w;
			capture_in_frame_y2f = in_h;

			if(frame_in_capture_x1f < 0) { capture_in_frame_x1f -= frame_in_capture_x1f; frame_in_capture_x1f = 0; }
			if(frame_in_capture_y1f < 0) { capture_in_frame_y1f -= frame_in_capture_y1f; frame_in_capture_y1f = 0; }
			if(frame_in_capture_x2f > capture_w) { capture_in_frame_x2f -= frame_in_capture_x2f - capture_w; frame_in_capture_x2f = capture_w; }
			if(frame_in_capture_y2f > capture_h) { capture_in_frame_y2f -= frame_in_capture_y2f - capture_h; frame_in_capture_y2f = capture_h; }

			frame_in_capture_x1 = (int)frame_in_capture_x1f;
			frame_in_capture_y1 = (int)frame_in_capture_y1f;
			frame_in_capture_x2 = (int)frame_in_capture_x2f;
			frame_in_capture_y2 = (int)frame_in_capture_y2f;

			capture_in_frame_x1 = (int)capture_in_frame_x1f;
			capture_in_frame_y1 = (int)capture_in_frame_y1f;
			capture_in_frame_x2 = (int)capture_in_frame_x2f;
			capture_in_frame_y2 = (int)capture_in_frame_y2f;

			frame_resized = 0;
		}
	}
return 0;
}

int VideoDevice::unmap_v4l_shmem()
{
	if(capture_buffer)
	{
		if(shared_memory)
			munmap(capture_buffer, capture_params.size);
		else
			delete capture_buffer;
	}
return 0;
}

int VideoDevice::unmap_v4l2_shmem()
{
	if(capture_buffer)
	{
		delete capture_buffer;
		capture_buffer = 0;
	}
	else
	if(v4l2_buffer_list)
	{
		for(int i = 0; i < v4l2_buffers.count; i++)
		{
			if((int)v4l2_buffer_list[i].data != -1)
				munmap(v4l2_buffer_list[i].data, 
					   v4l2_buffer_list[i].vidbuf.length);
		}
		delete v4l2_buffer_list;
		v4l2_buffer_list = 0;
	}
return 0;
}

int VideoDevice::close_v4l()
{
	unmap_v4l_shmem();
	if(input_fd != -1) close(input_fd);
return 0;
}

int VideoDevice::close_v4l2()
{
	if(v4l2_buffer_list)
	{
		int temp = V4L2_BUF_TYPE_CAPTURE;
		if(ioctl(input_fd, VIDIOC_STREAMOFF, &temp) < 0)
			perror("VideoDevice::close_v4l2 VIDIOC_STREAMOFF");
	}
	unmap_v4l2_shmem();
	if(input_fd != -1) close(input_fd);
return 0;
}

int VideoDevice::init_video4linux()
{
	int i;

	close_v4l();
	input_fd = open(in_config->v4l_in_device, O_RDWR);

	if(input_fd < 0)
		perror("VideoDevice::init_video4linux");
	else
	{
		set_cloexec_flag(input_fd, 1);
		if(ioctl(input_fd, VIDIOCGWIN, &window_params) < 0)
			perror("VideoDevice::init_video4linux VIDIOCGWIN");
		window_params.x = 0;
		window_params.y = 0;
		window_params.width = capture_w;
		window_params.height = capture_h;
		window_params.chromakey = 0;
		window_params.flags = 0;
		window_params.clipcount = 0;
		if(ioctl(input_fd, VIDIOCSWIN, &window_params) < 0)
			perror("VideoDevice::init_video4linux VIDIOCSWIN");
		if(ioctl(input_fd, VIDIOCGWIN, &window_params) < 0)
			perror("VideoDevice::init_video4linux VIDIOCGWIN");

		capture_w = window_params.width;
		capture_h = window_params.height;

		set_picture(-1, -1, -1, -1, -1);

		if(ioctl(input_fd, VIDIOCGMBUF, &capture_params) < 0)
			perror("VideoDevice::init_video4linux VIDIOCGMBUF");

//printf("Got %d frames\n", capture_params.frames);fflush(stdout);
		capture_buffer = (char*)mmap(0, 
			capture_params.size, 
			PROT_READ|PROT_WRITE, 
			MAP_SHARED, 
			input_fd, 
			0);

		capture_frame_number = 0;

		if(capture_buffer < 0)
		{
// Use read instead.
			perror("VideoDevice::init_video4linux mmap");
			shared_memory = 0;
			capture_buffer = new char[capture_params.size];
		}
		else
		{
// Get all frames capturing
			shared_memory = 1;
			for(i = 0; i < MIN(in_config->capture_length, capture_params.frames); i++)
				capture_frame(i);
		}
	}
return 0;
}

int VideoDevice::init_video4linux2()
{
	int i;
	v4l2_params.type = V4L2_BUF_TYPE_CAPTURE;

	close_v4l2();
	input_fd = open(in_config->v4l_in_device, O_RDONLY);

	if(input_fd < 0)
		perror("VideoDevice::init_video4linux2");
	else
	{
		struct v4l2_capability cap;

		set_cloexec_flag(input_fd, 1);
		if(ioctl(input_fd, VIDIOC_QUERYCAP, &cap))
			perror("VideoDevice::init_video4linux2 VIDIOC_QUERYCAP");

		v4l2_parm.type = V4L2_BUF_TYPE_CAPTURE;
// Set up frame rate
		if(ioctl(input_fd, VIDIOC_G_PARM, &v4l2_parm))
			perror("VideoDevice::init_video4linux2 VIDIOC_G_PARM");

		if(v4l2_parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
		{
			v4l2_parm.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
			v4l2_parm.parm.capture.timeperframe = (unsigned long)((float)1 / frame_rate * 10000000);
			if(ioctl(input_fd, VIDIOC_S_PARM, &v4l2_parm))
				perror("VideoDevice::init_video4linux2 VIDIOC_S_PARM");

			if(ioctl(input_fd, VIDIOC_G_PARM, &v4l2_parm))
				perror("VideoDevice::init_video4linux2 VIDIOC_G_PARM");
		}

// Set up data format
		if(ioctl(input_fd, VIDIOC_G_FMT, &v4l2_params) < 0)
			perror("VideoDevice::init_video4linux2 VIDIOC_G_FMT");
		v4l2_params.fmt.pix.width = capture_w;
		v4l2_params.fmt.pix.height = capture_h;
		v4l2_params.fmt.pix.depth = 24;
		v4l2_params.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
		v4l2_params.fmt.pix.flags |= V4L2_FMT_FLAG_INTERLACED;
		if(ioctl(input_fd, VIDIOC_S_FMT, &v4l2_params) < 0)
			perror("VideoDevice::init_video4linux2 VIDIOC_S_FMT");
		if(ioctl(input_fd, VIDIOC_G_FMT, &v4l2_params) < 0)
			perror("VideoDevice::init_video4linux2 VIDIOC_G_FMT");

		capture_w = v4l2_params.fmt.pix.width;
		capture_h = v4l2_params.fmt.pix.height;

// Set up buffers
		if(cap.flags & V4L2_FLAG_STREAMING)
		{
// Can use streaming
			shared_memory = 1;
			v4l2_buffers.count = in_config->capture_length;
			v4l2_buffers.type = V4L2_BUF_TYPE_CAPTURE;
			if(ioctl(input_fd, VIDIOC_REQBUFS, &v4l2_buffers) < 0)
				perror("VideoDevice::init_video4linux2 VIDIOC_REQBUFS");

			v4l2_buffer_list = new struct tag_vimage[v4l2_buffers.count];
			for(i = 0; i < v4l2_buffers.count; i++)
			{
				v4l2_buffer_list[i].vidbuf.index = i;
				v4l2_buffer_list[i].vidbuf.type = V4L2_BUF_TYPE_CAPTURE;
				if(ioctl(input_fd, VIDIOC_QUERYBUF, &v4l2_buffer_list[i].vidbuf) < 0)
					perror("VideoDevice::init_video4linux2 VIDIOC_QUERYBUF");
//printf("%d\n", v4l2_buffer_list[i].vidbuf.length);
				v4l2_buffer_list[i].data = (char*)mmap(0, 
								v4l2_buffer_list[i].vidbuf.length, 
								PROT_READ,
				    		  	MAP_SHARED, 
								input_fd, 
				    		  	v4l2_buffer_list[i].vidbuf.offset);

				if((int)v4l2_buffer_list[i].data == -1)
					perror("VideoDevice::init_video4linux2 mmap");
			}
// Start all frames capturing
			for(i = 0; i < v4l2_buffers.count; i++)
			{
				if(ioctl(input_fd, VIDIOC_QBUF, &v4l2_buffer_list[i].vidbuf) < 0)
					perror("VideoDevice::init_video4linux2 VIDIOC_QBUF");
			}
			if(ioctl(input_fd, VIDIOC_STREAMON, &v4l2_buffer_list[0].vidbuf.type) < 0)
				perror("VideoDevice::init_video4linux2 VIDIOC_STREAMON");
		}
		else
		{
// Need to use read()
			shared_memory = 0;
			capture_buffer = new char[v4l2_params.fmt.pix.sizeimage];
		}

		capture_frame_number = 0;
	}
return 0;
}

int VideoDevice::capture_frame(int capture_frame_number)
{
	if(in_config->video_in_driver == VIDEO4LINUX)
	{
		struct video_mmap params;
		params.frame = capture_frame_number;
		params.width = capture_w;
		params.height = capture_h;
// Required to actually set the palette.
		params.format = VIDEO_PALETTE_RGB24;
// Tells the driver the buffer is available for writing
		ioctl(input_fd, VIDIOCMCAPTURE, &params);
	}
	else
	if(in_config->video_in_driver == VIDEO4LINUX2)
	{
		if(ioctl(input_fd, VIDIOC_QBUF, &v4l2_buffer_list[capture_frame_number]) < 0)
			perror("VideoDevice::capture_frame VIDIOC_QBUF");
	}
return 0;
}

int VideoDevice::set_latency_counter(int value)
{
	latency_counter = value;
return 0;
}

int VideoDevice::wait_v4l_frame()
{
	if(ioctl(input_fd, VIDIOCSYNC, &capture_frame_number))
		perror("VideoDevice::wait_v4l_frame VIDIOCSYNC");
return 0;
}

int VideoDevice::wait_v4l2_frame()
{
	fd_set rdset;
	struct timeval timeout;
	FD_ZERO(&rdset);
	FD_SET(input_fd, &rdset);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if(select(input_fd + 1, &rdset, NULL, NULL, &timeout) <= 0)
		printf("VideoDevice::wait_v4l2_frame select error\n");

	struct v4l2_buffer tempbuf;
	tempbuf.type = v4l2_buffer_list[0].vidbuf.type;
	if(ioctl(input_fd, VIDIOC_DQBUF, &tempbuf))
		perror("VideoDevice::wait_v4l2_frame VIDIOC_DQBUF");
return 0;
}

int VideoDevice::read_v4l_frame(VFrame *frame)
{
	frame_to_vframe(frame, (unsigned char*)capture_buffer + capture_params.offsets[capture_frame_number]);
return 0;
}

int VideoDevice::read_v4l2_frame(VFrame *frame)
{
	if(v4l2_buffer_list && (int)v4l2_buffer_list[capture_frame_number].data != -1)
		frame_to_vframe(frame, (unsigned char*)v4l2_buffer_list[capture_frame_number].data);
return 0;
}

int VideoDevice::read_buffer(VFrame *frame)
{
	int result = 0;
	if(!capturing) return 0;

	if(in_config->video_in_driver == VIDEO4LINUX || in_config->video_in_driver == VIDEO4LINUX2)
	{
		if(latency_counter > 0)
		{
			frame->clear_frame();
			latency_counter--;
		}
	}

// Update the translation for all.
	if(frame_resized)
	{
		update_translation();
	}

	if(in_config->video_in_driver == VIDEO4LINUX || in_config->video_in_driver == VIDEO4LINUX2)
	{
		if(shared_memory)
		{
// If the next frame is different than the current frame, start it capturing.
//				if(next_frame(capture_frame_number) != capture_frame_number) capture_frame(next_frame(capture_frame_number));

// Reset the keepalive thread for video4linux
			keepalive->capturing = 1;

// Read the current frame
			if(in_config->video_in_driver == VIDEO4LINUX)
				wait_v4l_frame();
			else
			if(in_config->video_in_driver == VIDEO4LINUX2)
				wait_v4l2_frame();

			keepalive->capturing = 0;
			keepalive->reset_keepalive();

			if(in_config->video_in_driver == VIDEO4LINUX)
				read_v4l_frame(frame);
			else
			if(in_config->video_in_driver == VIDEO4LINUX2)
				read_v4l2_frame(frame);

// If the next frame is the same as the current frame start it capturing.
//				if(next_frame(capture_frame_number) == capture_frame_number) capture_frame(capture_frame_number);
// Start this frame capturing again
			capture_frame(capture_frame_number);
// Advance the frame to capture.
			capture_frame_number = next_frame(capture_frame_number);
		}
		else
		{
			if(in_config->video_in_driver == VIDEO4LINUX && capture_buffer)
				read(input_fd, capture_buffer, capture_params.size);
			else
			if(in_config->video_in_driver == VIDEO4LINUX2 && capture_buffer)
				read(input_fd, capture_buffer, v4l2_params.fmt.pix.sizeimage);
		}
	}
	else
	if(in_config->video_in_driver == SCREENCAPTURE)
	{
		capture_bitmap->capture_frame(frame, input_x, input_y);
	}
	else
	if(input_base)
	{
// Reset the keepalive thread
		if(keepalive) keepalive->capturing = 1;
		result = input_base->read_buffer(frame);
		if(keepalive)
		{
			keepalive->capturing = 0;
			keepalive->reset_keepalive();
		}
		return result;
	}

	return 0;
return 0;
}



typedef struct
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
} InPixel;

int VideoDevice::frame_to_vframe(VFrame *frame, unsigned char *input)
{
// Check boundaries
	if(frame_in_capture_x1 < frame_in_capture_x2 &&
		frame_in_capture_y1 < frame_in_capture_y2 &&
		capture_in_frame_x1 < capture_in_frame_x2 &&
		capture_in_frame_y1 < capture_in_frame_y2)
	{
		if(in_config->video_in_driver == VIDEO4LINUX || in_config->video_in_driver == VIDEO4LINUX2)
		{
			register int i, j, k, l;
			InPixel *row_in;
			unsigned char *row_out_start, *row_out_end;
			int bytes_per_row;
			int output_byte1 = capture_in_frame_x1 * 3;
			int output_byte2 = capture_in_frame_x2 * 3;
			int output_row_bytes = frame->get_w() * 3;
			unsigned char *rows_out = frame->get_data();

			if(in_config->video_in_driver == VIDEO4LINUX)
				bytes_per_row = window_params.width * 3;
			else
			if(in_config->video_in_driver == VIDEO4LINUX2)
				bytes_per_row = v4l2_params.fmt.pix.width * 3;

			for(i = frame_in_capture_y1, j = capture_in_frame_y1; 
				j < capture_in_frame_y2; 
				i++, j++)
			{
				row_in = (InPixel*)(input + bytes_per_row * i) + frame_in_capture_x1;
				row_out_start = rows_out + j * output_row_bytes + output_byte1;
				row_out_end = row_out_start + output_byte2 - output_byte1;

				while(row_out_start < row_out_end)
				{
// Change this if you ever get a video card on a big endian box
					*row_out_start++ = row_in->red;
					*row_out_start++ = row_in->green;
					*row_out_start++ = row_in->blue;
					row_in++;
				}
			}
		}
	}
return 0;
}


int VideoDevice::next_frame(int previous_frame)
{
	int result = previous_frame + 1;
	
	if(in_config->video_in_driver == VIDEO4LINUX)
	{
		if(result >= MIN(in_config->capture_length, capture_params.frames)) result = 0;
	}
	else
	if(in_config->video_in_driver == VIDEO4LINUX2)
	{
		if(result >= v4l2_buffers.count) result = 0;
	}
	return result;
return 0;
}

// ================================= OUTPUT ==========================================



int VideoDevice::open_output(VideoConfig *config, 
					float rate, 
					int out_w, 
					int out_h, 
					int output_format,
					int slippery)
{
	w = 1;
	*this->out_config = *config;
	this->out_w = out_w;
	this->out_h = out_h;
	this->orate = rate;
	this->output_format = output_format;
	this->slippery = slippery;

	switch(out_config->video_out_driver)
	{
		case PLAYBACK_LML:
			output_base = new VDeviceLML(this);
			break;

		case PLAYBACK_X11:
			output_base = new VDeviceX11(this);
			break;
	}

	if(output_base->open_output())
	{
		delete output_base;
		output_base = 0;
	}

	if(output_base) return 0;
	return 1;
return 0;
}

int VideoDevice::start_playback()
{
// arm buffer before doing this
	is_playing_back = 1;
	interrupt = 0;

	if(output_base) return output_base->start_playback();
	return 1;
return 0;
}

int VideoDevice::stop_playback()
{
	if(output_base) output_base->stop_playback();
	is_playing_back = 0;
	interrupt = 0;
	return 0;
return 0;
}

int VideoDevice::interrupt_playback()
{
	interrupt = 1;
	return 0;
return 0;
}

ArrayList<int>* VideoDevice::get_render_strategies()
{
	if(output_base) return output_base->get_render_strategies();
	return 0;
}

int VideoDevice::write_buffer(VFrame *output)
{
	if(output_base) return output_base->write_buffer(output);
	return 1;
return 0;
}

int VideoDevice::output_visible()
{
	if(output_base) return output_base->output_visible();
return 0;
}

BC_Bitmap* VideoDevice::get_bitmap()
{
	if(output_base) return output_base->get_bitmap();
}
