#include "filehtal.h"
#include "mpeg.h"
#include "mpegwindow.h"
#include "overlayframe.h"
#include "vframe.h"
#include "strategies.inc"

int main(int argc, char *argv[])
{
	MpegMain *plugin;

	plugin = new MpegMain(argc, argv);
	plugin->initialize();
	plugin->plugin_run();
	delete plugin;
}

MpegMain::MpegMain(int argc, char *argv[])
 : PluginIOClient(argc, argv)
{
	reset_parameters();
}

MpegMain::~MpegMain()
{
	close_file();
	delete athread;
	delete vthread;
	if(!adefaults) delete adefaults;
	if(!vdefaults) delete vdefaults;
}

int MpegMain::initialize()
{
	athread = new MpegAudioThread(this);
	vthread = new MpegVideoThread(this);
	return 0;
}

int MpegMain::reset_parameters()
{
	overlayer = 0;
	rgb24_frame = 0;
	rgb24_rows = 0;
	temp_frame = 0;
	temp_frame_buffer = 0;
	audio_allocated = 0;
	audio_buffer = 0;
	current_channel = 0;
	current_layer = 0;
	current_sample = 0;
	current_frame = 0;
	seek_sample = 0;
	seek_frame = 0;
	cpus = 1;
	mpeg = 0;
	interlaced = 0;
	adefaults = 0;
	vdefaults = 0;
	temp_audio = 0;
	temp_video = 0;
	video_encoder = 0;
	audio_encoder = 0;

	abitrate = 384;
	vbitrate = 5000;
	video_layer = 1;
	interlaced = 0;
	return 0;
}

const char* MpegMain::plugin_title() { return "MPEG"; }
int MpegMain::plugin_is_audio() { return 1; }
int MpegMain::plugin_is_video() { return 1; }

int MpegMain::get_audio_parameters()
{
	load_defaults();
	athread->start();
	return 0;
}

int MpegMain::get_video_parameters()
{
	load_defaults();
	vthread->start();
	return 0;
}

int MpegMain::interrupt_aparameters()
{
	if(athread->running) athread->window->set_done(0);
	return 0;
}

int MpegMain::interrupt_vparameters()
{
	if(vthread->running) vthread->window->set_done(0);
	return 0;
}

int MpegMain::load_defaults()
{
	char directory[1024];
	if(adefaults) return 0;
	
	sprintf(directory, "%smpegaudio.rc", BCASTDIR);
	adefaults = new Defaults(directory);
	sprintf(directory, "%smpegvideo.rc", BCASTDIR);
	vdefaults = new Defaults(directory);

	adefaults->load();
	vdefaults->load();

	abitrate = adefaults->get("ABITRATE", abitrate);
	vbitrate = vdefaults->get("VBITRATE", vbitrate);
	interlaced = vdefaults->get("INTERLACED", interlaced);
	video_layer = vdefaults->get("VIDEO_LAYER", video_layer);
	return 0;
}

int MpegMain::save_adefaults()
{
	adefaults->update("ABITRATE", abitrate);
	adefaults->save();
	return 0;
}

int MpegMain::save_vdefaults()
{
	vdefaults->update("VBITRATE", vbitrate);
	vdefaults->update("INTERLACED", interlaced);
	vdefaults->update("VIDEO_LAYER", video_layer);
	vdefaults->save();
	return 0;
}

int MpegMain::check_header(char *path)
{
	int result = 0;
	result = mpeg3_check_sig(path);
//printf("MpegMain::check_header %d %s\n", result, path);
	return result;
}

int MpegMain::open_file(char *path, int rd, int wr)
{
	strcpy(this->path, path);

// Read only
	if(rd && !wr)
	{
		int error = 0;
		mpeg = mpeg3_open(path, &error);
		mpeg3_set_cpus(mpeg, PluginIOClient::cpus);
		//mpeg3_set_mmx(mpeg, 0);
		if(!mpeg) return 1;
		read_header();
	}
	else
// Write only
	if(wr && !rd)
	{
		char string[1024];
		load_defaults();

// Create temp filenames
		if(has_audio)
		{
			temp_audio = new char*[channels];
			for(int i = 0; i < channels; i++)
			{
				sprintf(string, "%s.a%d", path, i);
				temp_audio[i] = new char[strlen(string) + 1];
				strcpy(temp_audio[i], string);
			}
		}

		if(has_video)
		{
			temp_video = new char*[layers];
			for(int i = 0; i < layers; i++)
			{
				sprintf(string, "%s.v%d", path, i);
				temp_audio[i] = new char[strlen(string) + 1];
				strcpy(temp_audio[i], string);
			}
		}
	}
	return 0;
}


int MpegMain::close_file()
{
	int i;
	if(adefaults) delete adefaults;
	if(vdefaults) delete vdefaults;
	
	if(temp_audio)
	{
		for(i = 0; i < channels; i++)
			delete [] temp_audio[i];
		
		delete [] temp_audio;
	}

	if(temp_video)
	{
		for(i = 0; i < layers; i++)
			delete [] temp_video[i];
		
		delete [] temp_video;
	}

	if(temp_frame)
	{
		delete temp_frame;
		delete temp_frame_buffer;
	}

	if(rgb24_frame)
	{
		delete rgb24_frame;
		delete rgb24_rows;
	}

	if(overlayer)
	{
		delete overlayer;
	}

	if(mpeg) mpeg3_close(mpeg);

	if(audio_buffer) delete audio_buffer;

	reset_parameters();
	return 0;
}

int MpegMain::read_header()
{
	int i, streams;
	has_audio = mpeg3_has_audio(mpeg);
	if(has_audio)
	{
		streams = mpeg3_total_astreams(mpeg);
		sample_rate = mpeg3_sample_rate(mpeg, 0); // They better all have the same sample rate
		channels = 0;
		for(i = 0; i < streams; i++)
			channels += mpeg3_audio_channels(mpeg, i);
	}
	has_video = mpeg3_has_video(mpeg);
	if(has_video)
	{
		layers = mpeg3_total_vstreams(mpeg);
		frame_rate = mpeg3_frame_rate(mpeg, 0); // They better all have the same frame rate
		width = mpeg3_video_width(mpeg, 0);
		height = mpeg3_video_height(mpeg, 0);
	}
	return 0;
}

long MpegMain::get_alength()
{
	return mpeg3_audio_samples(mpeg, 0);
}

long MpegMain::get_vlength()
{
	return mpeg3_video_frames(mpeg, 0);
}

int MpegMain::seek_end()
{
	current_sample = get_alength();
	current_frame = get_vlength();
	return 0;
}

int MpegMain::seek_start()
{
	current_sample = 0;
	current_frame = 0;
	return 0;
}

long MpegMain::get_aposition()
{
	return current_sample;
}

long MpegMain::get_vposition()
{
	return current_frame;
}

int MpegMain::set_aposition(long sample)
{
	if(current_sample != sample)
	{
		current_sample = sample;
		seek_sample = 1;
	}
	return 0;
}

int MpegMain::set_vposition(long frame)
{
	if(current_frame != frame)
	{
		current_frame = frame;
		seek_frame = 1;
	}
	return 0;
}

int MpegMain::set_channel(int channel)
{
	current_channel = channel;
	return 0;
}

int MpegMain::set_layer(int layer)
{
	current_layer = layer;
	return 0;
}

int MpegMain::read_samples(float *buffer, long len, 
	int feather, 
	long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	int result;
	int stream, stream_channel;

	for(stream = 0, stream_channel = current_channel; 
		stream < mpeg3_total_astreams(mpeg) && stream_channel >= mpeg3_audio_channels(mpeg, stream);
		stream_channel -= mpeg3_audio_channels(mpeg, stream), stream++)
	;

	if(seek_sample) mpeg3_set_sample(mpeg, current_sample, stream);
	seek_sample = 0;

// Allocate a new buffer
	if(audio_allocated < len)
	{
		delete audio_buffer;
		audio_buffer = 0;
	}
	if(!audio_buffer)
	{
		audio_buffer = new float[len];
	}

// Overlay from the buffer
	if(feather)
	{
		float feather_current;     // input position for right feather
		float current_gain;
		long output_current = 0;  // position in output buffer
		float *audio_ptr = audio_buffer;

// Temp buffer
		result = mpeg3_read_audio(mpeg, audio_buffer, 0, stream_channel, len, stream);

// Left feather
		for(feather_current = 0; feather_current < lfeather_len; 
			output_current++, feather_current++)
		{
			current_gain = lfeather_gain + lfeather_slope * feather_current;
			buffer[output_current] = (buffer[output_current] * (1 - current_gain)) + *audio_ptr++ * current_gain;
		}

// Center
		for( ; output_current < len; output_current++)
		{
			buffer[output_current] += *audio_ptr++;
		}
	}
	else
	{
		result = mpeg3_read_audio(mpeg, buffer, 0, current_channel, len, stream);
	}
	
	current_sample += len;
	return result;
}

// Read frame for drawing and store locally
int MpegMain::read_frame_ptr(int use_alpha, 
		int use_float, 
		long &shared_id, 
		long &shared_size)
{
	int result = 0;

	if(!temp_frame)
	{
		temp_frame_buffer = new PluginBuffer(width * height + 1, sizeof(VPixel));
		temp_frame = new VFrame((unsigned char*)temp_frame_buffer->get_data(), width, height);
	}

	if(seek_frame) mpeg3_set_frame(mpeg, current_frame, current_layer);
	seek_frame = 0;

#if (VMAX == 65535)
	result = mpeg3_read_frame(mpeg, 
				(VWORD**)temp_frame->get_rows(), 
				0, 
				0, 
				width, 
				height, 
				width, 
				height, 
				MPEG3_RGBA16161616,
				current_layer);
#else
	result = mpeg3_read_frame(mpeg, 
				(VWORD**)temp_frame->get_rows(), 
				0, 
				0, 
				width, 
				height, 
				width, 
				height, 
				MPEG3_RGBA8888,
				current_layer);
#endif

	shared_id = temp_frame_buffer->get_id();
	shared_size = width * height;
	current_frame++;
	return result;
}

// Read frame for overlaying
int MpegMain::read_frame(VFrame *frame, 
	float in_x1, float in_y1, float in_x2, float in_y2,
	float out_x1, float out_y1, float out_x2, float out_y2, 
	int alpha, int use_alpha, int use_float, int interpolate)
{
	int result, i;

	if(!rgb24_frame)
	{
		rgb24_frame = new unsigned char[width * height * 3 + 4];
		rgb24_rows = new unsigned char*[height];
		for(i = 0; i < height; i++)
			rgb24_rows[i] = &rgb24_frame[width * i * 3];
	}

	if(seek_frame) mpeg3_set_frame(mpeg, current_frame, current_layer);
	seek_frame = 0;

	result = mpeg3_read_frame(mpeg, 
				rgb24_rows, 
				0, 
				0, 
				width, 
				height, 
				width, 
				height, 
				MPEG3_RGB888,
				current_layer);

	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, interpolate, NORMAL))
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, interpolate, NORMAL, OVERLAY_RGB);
	}

	overlayer->overlay(frame, rgb24_frame,
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha, width, height);
	current_frame++;

	return result;
}

// Read frame for direct copy playback
int MpegMain::read_raw_frame(VFrame *frame)
{
	int result, i;
	unsigned char **rows;
	rows = new unsigned char*[height];
	for(i = 0; i < height; i++) rows[i] = frame->get_rows()[i];

	if(seek_frame) mpeg3_set_frame(mpeg, current_frame, current_layer);
	seek_frame = 0;

	result = mpeg3_read_frame(mpeg, 
				rows, 
				0, 
				0, 
				width, 
				height, 
				width, 
				height, 
				MPEG3_RGB888,
				current_layer);
	current_frame++;

	delete [] rows;
	return result;
}

int MpegMain::read_rgb24_frame(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	int result = 0;

	if(!rgb24_frame)
	{
		rgb24_frame = new unsigned char[width * height * 3];
	}

//	result = mpeg3_read_frame(mpeg, rgb24_frame);

	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, interpolate, NORMAL))
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, interpolate, NORMAL, OVERLAY_RGB);
	}

	overlayer->overlay(frame, rgb24_frame,
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha, width, height);

	return result;
}

int MpegMain::write_samples(float **buffer, long len)
{
	return 0;
}

int MpegMain::write_frame(VFrame *frame, int use_alpha, int use_float)
{
	return 0;
}

int MpegMain::read_raw_frame_possible()
{
	return 1;
}

int MpegMain::test_strategy(int strategy)
{
	if(strategy == VRENDER_RGB888)
		return 1;
	
	if(strategy == VRENDER_VPIXEL)
		return 1;

	return 0;
}
