#include <string.h>
#include "assets.h"
#include "byteorder.h"
#include "file.h"
#include "filebase.h"
#include "filehtal.h"
#include "filejpeg.h"
#include "filejpeglist.h"
#include "filemov.h"
#include "filepcm.h"
#include "fileplugin.h"
#include "filepng.h"
#include "filethread.h"
#include "filetiff.h"
#include "formatwindow.h"
#include "pluginioserver.h"
#include "pluginserver.h"
#include "stringfile.h"
#include "vframe.h"



File::File()
{
	file = 0;

	cpus = 1;
	audio_thread = 0;
	video_thread = 0;
	asset = 0;
	getting_video_options = 0;
	getting_audio_options = 0;
	aformat_window = 0;
	vformat_window = 0;
	aformat_plugin = 0;
	vformat_plugin = 0;
}

File::~File()
{
	if(getting_video_options)
	{
		if(vformat_window) vformat_window->set_done(0);
		if(vformat_plugin) vformat_plugin->interrupt_video_parameters();
		vformat_completion.lock();
		vformat_completion.unlock();
	}

	if(getting_audio_options)
	{
		if(aformat_window) aformat_window->set_done(0);
		if(aformat_plugin) aformat_plugin->interrupt_audio_parameters();
		aformat_completion.lock();
		aformat_completion.unlock();
	}

	close_file();
}

int File::get_audio_options(ArrayList<PluginServer*> *plugindb, Asset *asset, int *dither)
{
	getting_audio_options = 1;
	if(asset->format & 0x8000)
	{
// Use plugin
		if((asset->format ^ 0x8000) < plugindb->total)
		{
			aformat_completion.lock();
			aformat_plugin = new PluginIOServer(*(plugindb->values[asset->format ^ 0x8000]));
			aformat_plugin->open_plugin();
			aformat_plugin->get_audio_parameters();
			getting_audio_options = 0;
			aformat_plugin->close_plugin();
			delete aformat_plugin;
			aformat_plugin = 0;
			aformat_completion.unlock();
		}
	}
	else
	{
// Use built-in window
		aformat_completion.lock();
		aformat_window = new FormatAWindow(asset, dither);
		aformat_window->create_objects();
		aformat_window->run_window();
		getting_audio_options = 0;
		delete aformat_window;
		aformat_window = 0;
		aformat_completion.unlock();
	}
	return 0;
return 0;
}

int File::get_video_options(ArrayList<PluginServer*> *plugindb, Asset *asset, int recording)
{
	getting_video_options = 1;
	if(asset->format & 0x8000)
	{
// Use plugin
		if((asset->format ^ 0x8000) < plugindb->total)
		{
			vformat_completion.lock();
			vformat_plugin = new PluginIOServer(*(plugindb->values[asset->format ^ 0x8000]));
			vformat_plugin->open_plugin();
			vformat_plugin->get_video_parameters();
			getting_video_options = 0;
			vformat_plugin->close_plugin();
			delete vformat_plugin;
			vformat_plugin = 0;
			vformat_completion.unlock();
		}
	}
	else
	{
// Use built-in window
		vformat_completion.lock();
		vformat_window = new FormatVWindow(asset, recording);
		vformat_window->create_objects();
		vformat_window->run_window();
		getting_video_options = 0;
		delete vformat_window;
		vformat_window = 0;
		vformat_completion.unlock();
	}
	return 0;
}



int File::set_processors(int cpus)   // Set the number of cpus for certain codecs
{
	this->cpus = cpus;
return 0;
}

int File::set_preload(long size)
{
	this->playback_preload = size;
return 0;
}


int File::try_to_open_file(ArrayList<PluginServer*> *plugindb, Asset *asset, int rd, int wr)
{
	this->asset = asset;
	file = 0;

	switch(asset->format)
	{
// get the format now
// If you add another format to case 0, you also need to add another case for the
// file format #define.
		case 0:
			FILE *stream;
			if(!(stream = fopen(asset->path, "rb")))
			{
// file not found
				return 1;
			}
			char test[16];
			fread(test, 16, 1, stream);

			if(test[0] == 'R' && test[1] == 'I' && test[2] == 'F' && test[3] == 'F')
			{
// WAV file
				fclose(stream);
				file = new FileWAV(asset, this);
				int result = file->open_file(rd, wr);
				if(!result) file->read_header();
				return result;
				break;
			}
			else
			if(png_check_sig((unsigned char*)test, 8))
			{
// PNG file
				fclose(stream);
				file = new FilePNG(asset, this);
			}
			else
			if(test[6] == 'J' && test[7] == 'F' && test[8] == 'I' && test[9] == 'F')
			{
// JPEG file
				fclose(stream);
				file = new FileJPEG(asset, this);
			}
			else
			if(test[0] == 'J' && test[1] == 'P' && test[2] == 'E' && 
				test[3] == 'G' && test[4] == 'L' && test[5] == 'I' && 
				test[6] == 'S' && test[7] == 'T')
			{
// List of JPEG files
				fclose(stream);
				file = new FileJPEGList(asset, this);
			}
			else
			if(test[0] == 'I' && test[1] == 'I')
			{
// TIFF file
				fclose(stream);
				file = new FileTIFF(asset, this);
			}
			else
			if(test[0] == '<' && test[1] == 'H' && test[2] == 'T' && test[3] == 'A' && test[4] == 'L' && test[5] == '>')
			{
// HTAL file
				fclose(stream);
				return 3;
			}    // can't load project file
			else
// Plugin file
			{
				file = new FilePlugin(plugindb, asset, this);
				if(file->check_header())
				{
					fclose(stream);
					file->close_file();
					delete file;
					file = new FilePlugin(plugindb, asset, this);
				}
				else
				{
					delete file;
					file = 0;
					if(quicktime_check_sig(asset->path))
					{
// MOV file
// should be last because quicktime lacks a magic number
						fclose(stream);
						file = new FileMOV(asset, this);
					}
					else
					{
// PCM file
						fclose(stream);
						return 2;
					}   // need more info
				}
			}
			break;

// format already determined
		case WAV:
			file = new FileWAV(asset, this);
			break;

		case PCM:
			file = new FilePCM(asset, this);
			break;

		case PNG:
			file = new FilePNG(asset, this);
			break;

		case JPEG:
			file = new FileJPEG(asset, this);
			break;

		case JPEG_LIST:
			file = new FileJPEGList(asset, this);
			break;

		case FILE_TIFF:
			file = new FileTIFF(asset, this);
			break;

		case MOV:
			file = new FileMOV(asset, this);
			break;

// try plugins
		default:
			if((asset->format & 0x8000) &&
				(asset->format & 0x7fff) < plugindb->total)
			{
				file = new FilePlugin(plugindb, asset, this);
			}
			else
// non existant format
				return 1;
			break;
	}

	if(file->open_file(rd, wr))
	{
		delete file;
		file = 0;
	}

	if(file)
		return 0;
	else
		return 1;
return 0;
}

int File::close_file()
{
	stop_audio_thread();
	stop_video_thread();
	if(file) 
	{
		file->close_file();
		delete file;
	}

	file = 0;
	asset = 0;
return 0;
}

int File::start_audio_thread(long buffer_size, int total_buffers)
{
	audio_thread = new FileThread(this, 1, 0);
	audio_thread->start_writing(buffer_size, 0, total_buffers, 0);
return 0;
}

int File::start_video_thread(long buffer_size, int color_model, int total_buffers, int compressed)
{
	video_thread = new FileThread(this, 0, 1);
	video_thread->start_writing(buffer_size, 
		color_model, 
		total_buffers, 
		compressed);
return 0;
}

int File::stop_audio_thread()
{
	if(audio_thread)
	{
		audio_thread->stop_writing();
		delete audio_thread;
		audio_thread = 0;
	}
return 0;
}

int File::stop_video_thread()
{
	if(video_thread)
	{
		video_thread->stop_writing();
		delete video_thread;
		video_thread = 0;
	}
return 0;
}
// 
// int File::lock_buffer(int number)
// {
// 	if(audio_thread)
// 	{
// // lock the input lock
// 		audio_thread->input_lock[number].lock();
// 	}
// }

int File::lock_read()
{
	read_lock.lock();
return 0;
}

int File::unlock_read()
{
	read_lock.unlock();
return 0;
}

int File::set_channel(int channel) 
{ 
	if(file && channel < asset->channels)
		return file->set_channel(channel);
	else
		return 1;
return 0;
}

int File::set_layer(int layer) { file->set_layer(layer); return 0;
}

int File::set_dither() { file->set_dither(); return 0;
}

long File::get_length() { return file->get_audio_length(); }

long File::get_audio_length() { return file->get_audio_length(); }

long File::get_video_length(float base_framerate) 
{ 
	long result = file->get_video_length();
	if(result > 0)
		return (long)(file->get_video_length() / asset->frame_rate * base_framerate + 0.5); 
	else
		return -1;  // infinity
}

long File::get_position() { return file->get_audio_position(); }

long File::get_video_position(float base_framerate) 
{ 
	return (long)(file->get_video_position() / asset->frame_rate * base_framerate + 0.5); 
}

long File::get_audio_position() { return file->get_audio_position(); }

int File::seek_end() 
{ 
	if(file)
		file->seek_end(); 
return 0;
}

int File::seek_start() 
{ 
	if(file)
		file->seek_start(); 
return 0;
}

int File::set_position(long x) 
{ 
	if(file)
		file->set_audio_position(x); 
return 0;
}

int File::set_audio_position(long x) 
{ 
	if(file)
		file->set_audio_position(x); 
return 0;
}

int File::set_video_position(long x, float base_framerate) 
{ 
// Normalize frame rate
	if(file)
		file->set_video_position((long)((double)x / 
			base_framerate * 
			asset->frame_rate + 
			0.5));
return 0;
}

long File::get_memory_usage() 
{ 
	if(file)
		return file->get_memory_usage();
return 0;
}

int File::write_samples(float **buffer, 
		PluginBuffer *audio_ram, 
		long byte_offset, 
		long allocated_samples, 
		long len)
{ 
	int result;
	write_lock.lock();
	result = file->write_samples(buffer, audio_ram, byte_offset, allocated_samples, len);
	write_lock.unlock();
	return result;
return 0;
}

int File::write_frames(VFrame ***frames, 
		PluginBuffer *video_ram, 
		int len, 
		int use_alpha, 
		int use_float)
{
	int result;
	write_lock.lock();
	result = file->write_frames(frames, 
					video_ram, 
					len, 
					use_alpha, 
					use_float);
	write_lock.unlock();
	return result;
return 0;
}

int File::write_compressed_frame(VFrame *buffer)
{
	int result;
	write_lock.lock();
	result = file->write_compressed_frame(buffer);
	write_lock.unlock();
	return result;
return 0;
}


int File::write_audio_buffer(long len)
{
	if(audio_thread)
	{
		return audio_thread->write_buffer(len);
	}
	return 1;
return 0;
}

int File::write_video_buffer(long len, int use_alpha, int use_float, int compressed)
{
	if(video_thread)
	{
		return video_thread->write_buffer(len, use_alpha, use_float, compressed);
	}

	return 1;
return 0;
}

float** File::get_audio_buffer()
{
	if(audio_thread) return audio_thread->get_audio_buffer();
return;
}

VFrame*** File::get_video_buffer()
{
	if(video_thread) return video_thread->get_video_buffer();
return;
}


int File::read_samples(PluginBuffer *buffer, long offset, long len)
{ 
	return file->read_samples(buffer, offset, len); 
return 0;
}

int File::render_samples(PluginBuffer *buffer, long offset, long len, 
		long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	return file->read_samples(buffer, offset, len, 1, 
		lfeather_len, lfeather_gain, lfeather_slope); 
return 0;
}

int File::read_compressed_frame(VFrame *buffer)
{
	if(file)
		return file->read_compressed_frame(buffer);
	else return 1;
return 0;
}

long File::compressed_frame_size()
{
	if(file)
		return file->compressed_frame_size();
	else return 0;
}

// Read into an RGB frame for single copy playback.
int File::read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset)
{
	if(!file) return 1;

	if(!file->read_raw_frame_possible())
	{
// If the file can't produce an RGB frame we do a dual copy playback.
		VFrame *file_output = file->read_frame(0, 0);
		register int i;
		long length = frame->get_w() * frame->get_h();
		register unsigned char *output = frame->get_data();
		register VPixel *input = ((VPixel**)file_output->get_rows())[0];

		for(i = 0; i < length; i++)
		{
#if (VMAX == 65535)
			*output++ = input[i].r >> 8;
			*output++ = input[i].g >> 8;
			*output++ = input[i].b >> 8;
#else
			*output++ = input[i].r;
			*output++ = input[i].g;
			*output++ = input[i].b;
#endif
		}
	}
	else
		return file->read_raw_frame(frame, buffer, byte_offset);
return 0;
}

// Return a pointer to a frame in the video file for drawing purposes.
// The temporary frame is created by the file handler so that still frame
// files don't have to copy to a new buffer.
VFrame* File::read_frame(int use_alpha, int use_float)
{
	if(file)
		return file->read_frame(use_alpha, use_float);
	else
		return 0;
}

int File::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	if(file)
		return file->read_frame(frame, buffer, byte_offset, 
			in_x1, in_y1, in_x2, in_y2,
			out_x1, out_y1, out_x2, out_y2, 
			alpha, use_alpha, use_float, interpolate);
	else
		return 1;
return 0;
}


int File::can_copy_from(Asset *asset, int output_w, int output_h)
{
	if(file)
	{
		return file->can_copy_from(asset) &&
			asset->width == output_w &&
			asset->height == output_h;
	}
	else
		return 0;
return 0;
}

int File::get_render_strategy(ArrayList<int>* render_strategies)
{
	if(file)
	{
		return file->get_render_strategy(render_strategies);
	}
	return VRENDER_VPIXEL;
return 0;
}


// Fill in queries about formats when adding formats here.



int File::strtoformat(ArrayList<PluginServer*> *plugindb, char *format)
{
	if(!strcmp(format, "WAV")) return WAV;
	else
	if(!strcmp(format, "PCM")) return PCM;
	else
	if(!strcmp(format, "PNG")) return PNG;
	else
	if(!strcmp(format, "TIFF")) return FILE_TIFF;
	else
	if(!strcmp(format, "JPEG")) return JPEG;
	else
	if(!strcmp(format, JPEG_LIST_NAME)) return JPEG_LIST;
	else
	if(!strcmp(format, MOV_NAME)) return MOV;
	else
	{
		int i, result = -1;
		for(i = 0; i < plugindb->total; i++)
		{	
			if(plugindb->values[i]->fileio && 
				!strcmp(plugindb->values[i]->title, format))
			{
				result = i | 0x8000;
				break;
			}
		}
		if(result != -1) return result;
	}
	return 0;
return 0;
}

char* File::formattostr(ArrayList<PluginServer*> *plugindb, int format)
{
	switch(format)
	{
		case WAV:
			return "WAV";
			break;
		case PCM:
			return "PCM";
			break;
		case PNG:
			return "PNG";
			break;
		case JPEG:
			return "JPEG";
			break;
		case JPEG_LIST:
			return JPEG_LIST_NAME;
			break;
		case FILE_TIFF:
			return "TIFF";
			break;
		case MOV:
			return MOV_NAME;
			break;

		default:
			if(format & 0x8000 && 
				(format ^ 0x8000) < plugindb->total &&
				plugindb->values[format ^ 0x8000]->fileio)
			{
				return plugindb->values[format ^ 0x8000]->title;
			}
			else
				return "WAV";
			break;
	}
	return "";
}

int File::strtobits(char *bits)
{
	if(!strcmp(bits, "8")) return BITSLINEAR8;
	if(!strcmp(bits, "16")) return BITSLINEAR16;
	if(!strcmp(bits, "24")) return BITSLINEAR24;
	if(!strcmp(bits, "ULAW")) return BITSULAW;
	if(!strcmp(bits, "IMA4")) return BITSIMA4;
	if(!strcmp(bits, "WMX2")) return BITSWMX2;
	return 0;
return 0;
}

char* File::bitstostr(int bits)
{
//printf("File::bitstostr\n");
	switch(bits)
	{
		case BITSLINEAR8:
			return "8";
			break;
		case BITSLINEAR16:
			return "16";
			break;
		case BITSLINEAR24:
			return "24";
			break;
		case BITSULAW:
			return "ULAW";
			break;
		case BITSIMA4:
			return "IMA4";
			break;
		case BITSWMX2:
			return "WMX2";
			break;
	}
	return "";
}

#define DV_NAME "Direct copy DV"
#define PNG_NAME "PNG"
#define PNGA_NAME "PNG with Alpha"
#define RGB_NAME "Uncompressed RGB"
#define RGBA_NAME "Uncompressed RGBA"
#define YUV420_NAME "YUV 4:2:0 Planar"
#define YUV422_NAME "YUV 4:2:2 Packed"
#define YUV411_NAME "YUV 4:1:1 Planar"
#define YUV444_NAME "YUV 4:4:4 Planar"
#define YUVA4444_NAME "YUV 4:4:4:4 Planar alpha"
#define YUV444_10BIT_NAME "YUV 4:4:4 10 Bit Planar"
#define QTJPEG_NAME "JPEG Photo"
#define MJPA_NAME "Motion JPEG A"


char* File::strtocompression(char *string)
{
	if(!strcmp(string, DV_NAME)) return QUICKTIME_DV;
	if(!strcmp(string, PNG_NAME)) return QUICKTIME_PNG;
	if(!strcmp(string, PNGA_NAME)) return MOV_PNGA;
	if(!strcmp(string, RGB_NAME)) return QUICKTIME_RAW;
	if(!strcmp(string, RGBA_NAME)) return MOV_RGBA;
	if(!strcmp(string, YUV420_NAME)) return QUICKTIME_YUV420;
	if(!strcmp(string, YUV422_NAME)) return QUICKTIME_YUV422;
	if(!strcmp(string, QTJPEG_NAME)) return QUICKTIME_JPEG;
	if(!strcmp(string, MJPA_NAME)) return QUICKTIME_MJPA;
	return QUICKTIME_RAW;
}

char* File::compressiontostr(char *string)
{
	if(!strcmp(string, QUICKTIME_YUV4)) return "YUV 4:2:0 Packed";
	if(!strcmp(string, QUICKTIME_YUV420)) return YUV420_NAME;
	if(!strcmp(string, QUICKTIME_YUV422)) return YUV422_NAME;
	if(!strcmp(string, QUICKTIME_PNG)) return PNG_NAME;
	if(!strcmp(string, MOV_PNGA)) return PNGA_NAME;
	if(!strcmp(string, QUICKTIME_RAW)) return RGB_NAME;
	if(!strcmp(string, MOV_RGBA)) return RGBA_NAME;
	if(!strcmp(string, QUICKTIME_JPEG)) return QTJPEG_NAME;
	if(!strcmp(string, QUICKTIME_MJPA)) return MJPA_NAME;
	if(!strcmp(string, QUICKTIME_DV)) return DV_NAME;
	return "RGB";
}

int File::bytes_per_sample(int bits)
{
	switch(bits)
	{
		case BITSLINEAR8:
			return 1;
			break;
		case BITSLINEAR16:
			return 2;
			break;
		case BITSLINEAR24:
			return 3;
			break;
		case BITSULAW:
			return 1;
			break;
		case BITSIMA4:
			return 1;
			break;
		case BITSWMX2:
			return 1;
			break;
	}
	return 1;
return 0;
}

int File::supports_video(ArrayList<PluginServer*> *plugindb, char *format)
{
	int i;
	if( !strcmp(format, MOV_NAME) ||
		!strcmp(format, JPEG_LIST_NAME)) return 1;

	for(i = 0; i < plugindb->total; i++)
	{	
		if(plugindb->values[i]->fileio && 
			!strcmp(plugindb->values[i]->title, format))
		{
			if(plugindb->values[i]->video) return 1;
		}
	}

	return 0;
return 0;
}

int File::supports_audio(ArrayList<PluginServer*> *plugindb, char *format)
{
	int i;
	if( !strcmp(format, "WAV") ||
		!strcmp(format, "PCM") ||
		!strcmp(format, MOV_NAME) ||
		!strcmp(format, JPEG_LIST_NAME)) return 1;

	for(i = 0; i < plugindb->total; i++)
	{	
		if(plugindb->values[i]->fileio && 
			!strcmp(plugindb->values[i]->title, format))
		{
			if(plugindb->values[i]->audio) return 1;
		}
	}

	return 0;
return 0;
}
