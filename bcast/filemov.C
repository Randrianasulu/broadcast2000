#include <string.h>
#include "assets.h"
#include "colormodels.h"
#include "file.h"
#include "filemov.h"
#include "pluginbuffer.h"
#include "vframe.h"



FileMOV::FileMOV(Asset *asset, File *file)
 : FileBase(asset, file)
{
	reset_parameters();
	asset->format = MOV;
	asset->byte_order = 0;
	suffix_number = 0;
}

FileMOV::~FileMOV()
{
	close_file();
}

int FileMOV::reset_parameters_derived()
{
	file = 0;
	prev_track = 0;
	data = 0;
	quicktime_atracks = 0;
	quicktime_vtracks = 0;
	use_bcast_audio = 0;
	depth = 24;
	threads = 0;
	frames_correction = 0;
	samples_correction = 0;
return 0;
}


// Just create the Quicktime objects since this routine is also called
// for reopening.
int FileMOV::open_file(int rd, int wr)
{
	this->rd = rd;
	this->wr = wr;
	if(suffix_number == 0) strcpy(prefix_path, asset->path);

	if(!(file = quicktime_open(asset->path, rd, wr)))
	{
		perror("FileMOV::open_file");
		return 1;
	}

	quicktime_set_cpus(file, FileBase::file->cpus);
	if(!wr && rd) read_header();

	if(wr)
	{
		char audio_codec[5];
// Fix up the Quicktime file.
		quicktime_set_copyright(file, "Made with Broadcast 2000 for Linux");
		quicktime_set_info(file, "Quicktime for Linux");

		if(!asset->signed_ && asset->bits == BITSLINEAR8)
		{
			strcpy(audio_codec, QUICKTIME_RAW);
			use_bcast_audio = 1;
		}
		else
		if(asset->signed_ && asset->bits == BITSLINEAR8)
		{
			strcpy(audio_codec, QUICKTIME_TWOS);
			use_bcast_audio = 1;
		}
		else
		if(asset->bits == BITSULAW)
		{
			strcpy(audio_codec, QUICKTIME_ULAW);
		}
		else
		if(asset->bits == BITSIMA4)
		{
			strcpy(audio_codec, QUICKTIME_IMA4);
		}
		else
		if(asset->bits == BITSLINEAR16 || asset->bits == BITSLINEAR24)
		{
			strcpy(audio_codec, QUICKTIME_TWOS);
			asset->signed_ = 1;
			use_bcast_audio = 1;
		}

		if(asset->video_data)
		{
			char string[16];
// Set up the alpha channel compressors
			if(!strcmp(asset->compression, MOV_RGBA))
			{
				strcpy(string, QUICKTIME_RAW);
				depth = 32;
			}
			else
			if(!strcmp(asset->compression, MOV_PNGA))
			{
				strcpy(string, QUICKTIME_PNG);
				depth = 32;
			}
			else
			{
				strcpy(string, asset->compression);
				depth = 24;
			}

			quicktime_vtracks = quicktime_set_video(file, 
						asset->layers, 
						asset->width, 
						asset->height,
						asset->frame_rate,
						string);
			for(int i = 0; i < asset->layers; i++)
				quicktime_set_depth(file, depth, i);
		}

		if(asset->audio_data)
		{
			quicktime_atracks = quicktime_set_audio(file, 
					asset->channels, 
					asset->rate, 
					asset->bits, 
					audio_codec);
		}
	}

// set the compression parameters if there are any
	quicktime_set_jpeg(file, asset->quality, 0);
	return 0;
return 0;
}

int FileMOV::close_file_derived()
{
	if(file)
	{
		quicktime_set_framerate(file, asset->frame_rate);
		quicktime_close(file);
	}

	if(data)
	{
		delete data;
	}

	if(threads)
	{
		for(int i = 0; i < FileBase::file->cpus; i++)
		{
			threads[i]->stop_encoding();
			delete threads[i];
		}
		delete [] threads;
		threads = 0;
	}
	file = 0;
	data = 0;
	return 0;
return 0;
}

int FileMOV::reopen_file()
{
	FILE *test;
	int i, result = 0;
	int attempts = 0;

// Unfortunately the compression threads currently running need Quicktime
// for the compression.
	for(i = 0; i < FileBase::file->cpus && threads; i++)
	{
		threads[i]->quicktime_lock.lock();
	}

// Just want to close the Quicktime file but keep the frame buffers and threads.
	if(file)
	{
		quicktime_set_framerate(file, asset->frame_rate);
		quicktime_close(file);
		file = 0;
	}

	frames_correction += video_position;
	samples_correction += audio_position;
	video_position = 0;
	audio_position = 0;
	suffix_number++;
	sprintf(asset->path, "%s%03d", prefix_path, suffix_number);

	do{
		result = 0;
		if((test = fopen(asset->path, "r")))
		{
// 			file = 0;
// 			printf("FileMOV::reopen_file: %s existed when trying to defeat 32 bit overflow", asset->path);
// 			result = 1;
			fclose(test);
			remove(asset->path);
		}

		if(!result) result = open_file(rd, wr);

		if(result)
		{
			printf("FileMOV::reopen_file: Failed to open new file.\n");
		}
		else
		{
			printf("FileMOV::reopen_file: Opened a new file.\n");
		}
	}while(result && attempts++ < 10);

	for(i = 0; i < FileBase::file->cpus && threads; i++)
	{
		threads[i]->quicktime_lock.unlock();
	}
	return result;
return 0;
}

int FileMOV::test_length(int result)
{
	if(!file) return 1;
	int result2 = 0;

// 	result2 = quicktime_test_position(file);
// 	if(result2)
// 	{
// 		return reopen_file();
// 	}
	return result;
return 0;
}


int FileMOV::read_header()
{
	if(!file) return 0;
	asset->audio_data = quicktime_has_audio(file);
	if(asset->audio_data)
	{
		asset->channels = 0;
		int qt_tracks = quicktime_audio_tracks(file);
		for(int i = 0; i < qt_tracks; i++)
			asset->channels += quicktime_track_channels(file, i);
	
		asset->rate = quicktime_sample_rate(file, 0);
		asset->bits = quicktime_audio_bits(file, 0);
		char *compressor = quicktime_audio_compressor(file, 0);

		if(quicktime_supported_audio(file, 0))
		{
			asset->signed_ = 1;
		}

		if(match4(compressor, QUICKTIME_RAW)) 
		{
			asset->signed_ = 0;
			use_bcast_audio = 1;
		}
		else
		if(match4(compressor, QUICKTIME_TWOS)) 
		{
			use_bcast_audio = 1;
			asset->signed_ = 1;
		}
		else
// Fake the bits parameter for some codecs
		if(match4(compressor, QUICKTIME_ULAW)) asset->bits = BITSULAW;
		else
		if(match4(compressor, QUICKTIME_IMA4)) asset->bits = BITSIMA4;
		//else
		//printf("FileMOV::read_header: unsupported audio codec\n");
	}

// determine if the video can be read before declaring video data
	if(quicktime_has_video(file))
		if(quicktime_supported_video(file, 0))
		{
			asset->video_data = 1;
		}
		//else
		//	printf("FileMOV::read_header: unsupported video codec\n");

	if(asset->video_data)
	{
		depth = quicktime_video_depth(file, 0);
		asset->layers = quicktime_video_tracks(file);
		asset->width = quicktime_video_width(file, 0);
		asset->height = quicktime_video_height(file, 0);
// Don't want a user configured frame rate to get destroyed
		if(!asset->frame_rate)
			asset->frame_rate = quicktime_frame_rate(file, 0);

		char *compressor = quicktime_video_compressor(file, 0);
		asset->compression[0] = compressor[0];
		asset->compression[1] = compressor[1];
		asset->compression[2] = compressor[2];
		asset->compression[3] = compressor[3];
	}
return 0;
}

int FileMOV::can_copy_from(Asset *asset)
{
	if(!file) return 0;
	if(asset->format == JPEG_LIST && match4(this->asset->compression, QUICKTIME_JPEG))
		return 1;
	else
	if(asset->format == MOV && 
		(match4(asset->compression, this->asset->compression)))
		return 1;

	return 0;
return 0;
}

int FileMOV::read_raw_frame_possible()
{
	if(depth == 24) return 1;
	else return 0;
return 0;
}

int FileMOV::get_render_strategy(ArrayList<int>* render_strategies)
{
// Fastest
	if(search_render_strategies(render_strategies, VRENDER_MJPG) &&
			match4(asset->compression, QUICKTIME_MJPA))
		return VRENDER_MJPG;

// Slower
// 	if(search_render_strategies(render_strategies, VRENDER_BGR8880) &&
// 			quicktime_test_colormodel(file, 
// 					BC_BGR8888, 
// 					video_layer))
// 		return VRENDER_BGR8880;

// Slowest
	if(search_render_strategies(render_strategies, VRENDER_RGB888))
		return VRENDER_RGB888;

	return VRENDER_VPIXEL;
return 0;
}

long FileMOV::get_audio_length()
{
	if(!file) return 0;
	long result = quicktime_audio_length(file, 0) + samples_correction;

	return result;
}

long FileMOV::get_video_length()
{
	if(!file) return 0;
	long result = quicktime_video_length(file, 0) + frames_correction;
	return result;
}

int FileMOV::seek_end()
{
	if(!file) return 0;
	audio_position = get_audio_length() - samples_correction;
	video_position = get_video_length() - frames_correction;
	quicktime_seek_end(file);
	return 0;
}

int FileMOV::seek_start()
{
	if(!file) return 0;
	audio_position = 0;
	video_position = 0;	
	quicktime_seek_start(file);
	return 0;
}

long FileMOV::get_audio_position()
{
	if(!file) return 0;
	return quicktime_audio_position(file, 0) + samples_correction;
}

int FileMOV::set_audio_position(long x)
{
	if(!file) return 0;
	audio_position = x - samples_correction;
// quicktime sets positions for each track seperately so store position in audio_position
	if(audio_position >= 0 && audio_position < quicktime_audio_length(file, 0))
		return quicktime_set_audio_position(file, x, 0);
	else
		return 1;
return 0;
}

long FileMOV::get_video_position()
{
	if(!file) return 0;
	return quicktime_video_position(file, video_layer) + frames_correction;
}

int FileMOV::set_video_position(long x)
{
	if(!file) return 0;
	video_position = x - frames_correction;

	if(video_position < quicktime_video_length(file, video_layer))
	{
		return quicktime_set_video_position(file, x, video_layer);
	}
	else
	{
		video_position = quicktime_video_length(file, video_layer) - 1;
		return quicktime_set_video_position(file, video_position, video_layer);
		return 1;
	}
return 0;
}

int FileMOV::set_channel(int channel)
{
	if(!file) return 0;
	this->audio_channel = channel;
return 0;
}

int FileMOV::set_layer(int layer)
{
	if(!file) return 0;
	this->video_layer = layer;
return 0;
}

int FileMOV::write_samples(float **buffer, 
		PluginBuffer *audio_ram, 
		long byte_offset, 
		long allocated_samples, 
		long len)
{
	int i, j;
	long bytes;
	int result = 0, track_channels = 0;
	int chunk_size;

	if(!file) return 0;
	if(quicktime_supported_audio(file, 0) && !use_bcast_audio)
	{
// Use Quicktime's compressor. (Always used)
// Because of the way Quicktime's compressors work we want to limit the chunk
// size to speed up decompression but not create too many chunks.
		float **fake_buffer;

		fake_buffer = new float*[asset->channels];
		for(j = 0; j < len && !result; )
		{
			chunk_size = asset->rate;
			if(j + chunk_size * 2 > len) chunk_size = len - j;

			for(i = 0; i < asset->channels; i++)
			{
				fake_buffer[i] = &buffer[i][j];
			}

			result |= quicktime_encode_audio(FileMOV::file, 0, fake_buffer, chunk_size);
			result = test_length(result);
			j += chunk_size;
		}
		delete [] fake_buffer;
	}
	else
	{
// Use our own compressor.  (Not used)
// Originally intended to match the channels to track organization but
// today all channels are written to track 0.
		for(int i = 0, j = 0, track = 0; i < asset->channels && result == 0; i += track_channels, track++)
		{
			track_channels = quicktime_track_channels(file, track);

			get_audio_buffer(&audio_buffer_out, len, asset->bits, track_channels);

			bytes = samples_to_raw(audio_buffer_out, 
						buffer, 
						len, 
						asset->bits, 
						track_channels,
						asset->byte_order,
						asset->signed_);      // signed_

			result = quicktime_write_audio(file, audio_buffer_out, len, track);
		}
	}
	audio_position += len;
	return result;
return 0;
}

int FileMOV::write_frames(VFrame ***frames, 
		PluginBuffer *video_ram, 
		int len, 
		int use_alpha, 
		int use_float)
{
	long bytes;
	int i, j, k, result = 0;

	if(!file) return 0;

// Frames use alpha
	if(frames[0][0]->get_color_model() == VFRAME_VPIXEL)
	{
		get_video_buffer(&video_buffer_out, depth);
		get_row_pointers(video_buffer_out, &row_pointers_out, depth);
	}
	else
	{
// Frame from a video capture uses no alpha
	}


	for(i = 0; i < asset->layers && !result; i++)
	{
		if((match4(asset->compression, QUICKTIME_JPEG) || 
			match4(asset->compression, QUICKTIME_MJPA)) && 
			file->cpus > 1 && 
			!frames[0][0]->get_color_model() == VFRAME_VPIXEL)
		{
// Compress symmetrically on an SMP system.
			ThreadStruct *threadframe;
			int interlaced = match4(asset->compression, QUICKTIME_MJPA);

// Set up threads for symmetric compression.
			if(!threads)
			{
				threads = new FileMOVThread*[FileBase::file->cpus];
				for(j = 0; j < FileBase::file->cpus; j++)
				{
					threads[j] = new FileMOVThread(this, interlaced + 1);
					threads[j]->start_encoding();
				}
			}

// Set up the frame structures for each frame
			while(threadframes.total < len)
			{
				threadframes.append(threadframe = new ThreadStruct);
			}

// Load thread frame structures with new frames.
			for(j = 0; j < len; j++)
			{
				threadframes.values[j]->input = frames[i][j];
				threadframes.values[j]->completion_lock.lock();
			}
			total_threadframes = len;
			current_threadframe = 0;

// Start the threads compressing
			for(j = 0; j < FileBase::file->cpus; j++)
			{
				threads[j]->encode_buffer();
			}

// Write the frames as they're finished
			for(j = 0; j < len; j++)
			{
				threadframes.values[j]->completion_lock.lock();
				threadframes.values[j]->completion_lock.unlock();
				if(!result)
				{
					quicktime_write_frame(file, 
						threadframes.values[j]->output,
						threadframes.values[j]->output_size,
						i);
				}
			}
		}
		else
		for(j = 0; j < len && !result; j++)
		{
// Compress one at a time using the library.
			if(frames[i][j]->get_color_model() == VFRAME_VPIXEL) 
			{
				bytes = frame_to_raw(video_buffer_out, 
					frames[i][j], 
					asset->width, 
					asset->height, 
					use_alpha, 
					use_float, 
					depth == 24 ? FILEBASE_RAW : FILEBASE_RGBA);
			}
			else
			{
// Use no frame_to_raw transformation for captured video.  Encode the data directly.
				if(row_pointers_out) delete row_pointers_out;
				row_pointers_out = 0;
				get_row_pointers(frames[i][j]->get_data(), &row_pointers_out, depth);
			}

			result |= quicktime_encode_video(file, row_pointers_out, i);
			result = test_length(result);
		}
	}

	video_position += len;
	return result;
return 0;
}

int FileMOV::read_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	if(!file) return 0;

// forces read directly into frame
	read_raw(frame, in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, alpha, use_alpha, 
		use_float, interpolate);

	video_position++;
	return 0;
return 0;
}

VFrame* FileMOV::read_frame(int use_alpha, int use_float)
{
	if(!file) return 0;

// don't use alpha
// forces read into a temporary buffer
	read_raw(0, 
		0, 0, (float)asset->width, (float)asset->height, 
		0, 0, (float)asset->width, (float)asset->height, 
		VMAX, 0, use_float, 0);

	video_position++;
	return data;
}

long FileMOV::compressed_frame_size()
{
	if(!file) return 0;
	return quicktime_frame_size(file, video_position, video_layer);
}

int FileMOV::read_compressed_frame(VFrame *buffer)
{
	long result;
	if(!file) return 0;

	result = quicktime_read_frame(file, buffer->get_data(), video_layer);
	buffer->set_compressed_size(result);
	result = !result;
	video_position++;
	return result;
return 0;
}

int FileMOV::write_compressed_frame(VFrame *buffer)
{
	int result = 0;
	if(!file) return 0;

	result |= quicktime_write_frame(file, buffer->get_data(), buffer->get_compressed_size(), video_layer);
	result = test_length(result);

	video_position++;
	return result;
return 0;
}


int FileMOV::read_raw_frame(VFrame *frame, PluginBuffer *buffer, long byte_offset)
{
	int result = 0;
	if(!file) return 0;
	quicktime_set_video_position(file, video_position, video_layer);

	result = quicktime_decode_video(file, frame->get_rows(), video_layer);
	return result;
}


int FileMOV::read_raw(VFrame *frame, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate)
{
	long i, color_channels, bytes;

	if(!file) return 0;

// want to return a pointer to a temporary buffer for drawing on a timeline
	if(!frame && !data)
	{
// user wants frame stored in file handler's frame but data hasn't been created yet.
		frame = data = new VFrame(0, asset->width, asset->height);
	}
	if(!frame)
	{
// user wants frame stored in file handler's frame and data has been created.
		frame = data;
	}

	prev_layer = video_layer;
	prev_frame_position = video_position;

	get_video_buffer(&video_buffer_in, depth);
	get_row_pointers(video_buffer_in, &row_pointers_in, depth);

	quicktime_set_video_position(file, video_position, video_layer);
	quicktime_decode_video(file, row_pointers_in, video_layer);
	raw_to_frame(video_buffer_in, frame, 
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, alpha, 
		use_alpha, use_float, depth == 24 ? OVERLAY_RGB : OVERLAY_RGBA, 
		interpolate, asset->width, asset->height);
	return 0;
}

// Overlay samples
int FileMOV::read_samples(PluginBuffer *shared_buffer, long offset, long len, int feather, 
		long lfeather_len, float lfeather_gain, float lfeather_slope)
{
	int qt_track, qt_channel;
	float *buffer = (float*)shared_buffer->get_data() + offset;

	if(!file) return 0;

	if(quicktime_supported_audio(file, 0) && !use_bcast_audio)
	{
// Use Quicktime's decoder.
		if(feather)
		{
			get_float_buffer(&float_buffer, len);
			if(quicktime_decode_audio(file, NULL, float_buffer, len, audio_channel))
			{
				printf("FileMOV::read_samples: quicktime_decode_audio failed\n");
				return 1;
			}

// Feather the buffer.
			overlay_float_buffer(buffer, float_buffer, len, 
				lfeather_len, lfeather_gain, lfeather_slope);
		}
		else
// Don't feather the buffer.
		if(quicktime_decode_audio(file, 0, buffer, len, audio_channel))
		{
			printf("FileMOV::read_samples: quicktime_decode_audio failed\n");
			return 1;
		}
	}
	else
	{
// Use our own decoder.
		quicktime_channel_location(file, &qt_track, &qt_channel, audio_channel);
		get_audio_buffer(&audio_buffer_in, len, asset->bits, quicktime_track_channels(file, qt_track));

		if(read_raw(audio_buffer_in, len, qt_track))
		{
			printf("FileMOV::read_samples: read_raw failed\n");
			return 1;
		}

		raw_to_samples(buffer, 
					audio_buffer_in, 
					len, 
					asset->bits, 
					quicktime_track_channels(file, qt_track), 
					qt_channel, 
					feather, 
					lfeather_len, 
					lfeather_gain, 
					lfeather_slope);
	}

	audio_position += len;
	prev_len = len;
	return 0;
return 0;
}

// Read raw samples
int FileMOV::read_raw(char *buffer, long samples, int track)
{
	int result;

	if(!file) return 0;

// only load if position has changed or this is the first buffer
	if(prev_buffer_position == -1 || 
		prev_buffer_position != audio_position ||
		prev_len != samples ||
		track != prev_track)
	{
		long bytes = samples * quicktime_track_channels(file, track) * (FileBase::file->bytes_per_sample(asset->bits));
		prev_buffer_position = audio_position;
		prev_len = samples;
		prev_track = track;
		result = quicktime_read_audio(file, buffer, samples, track);

		if(!result) return 1;

		if(asset->bits == BITSLINEAR8 && !asset->signed_)       // fix the unsigned numbers to signed
		{    // change unsigned to signed
			int i = 0, len_ = bytes - 4;
			while(i < len_)
			{
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
				buffer[i++] ^= 128;
			}
			while(i < bytes)
			{
				buffer[i++] ^= 128;
			}
		}

//printf("FileMOV::read_raw internal_byte_order %d asset->byte_order %d\n", internal_byte_order, asset->byte_order);
		if((!internal_byte_order && asset->byte_order) || 
			(internal_byte_order && !asset->byte_order))
		{            // swap bytes if not intel
			swap_bytes(FileBase::file->bytes_per_sample(asset->bits), (unsigned char*)buffer, bytes);
		}
	}

	return 0;
return 0;
}

long FileMOV::get_memory_usage()
{
	int result = 4096;
	if(!file) return 0;

	if(data) result += asset->width * asset->height * sizeof(VPixel);
	if(video_buffer_in) result += asset->width * asset->height * 3;
	if(video_buffer_out) result += asset->width * asset->height * 3;
	if(audio_buffer_in) result += prev_bytes;
	if(audio_buffer_out) result += prev_bytes;
	return result;
}



// Parallel JPEG encoding

ThreadStruct::ThreadStruct()
{
	input = 0;
	output = 0;
	output_allocated = 0;
	output_size = 0;
}

ThreadStruct::~ThreadStruct()
{
	if(output) delete [] output;
}

void ThreadStruct::load_output(mjpeg_t *mjpeg)
{
	if(output_allocated < mjpeg_output_size(mjpeg))
	{
		delete [] output;
		output = 0;
	}
	if(!output)
	{
		output_allocated = mjpeg_output_size(mjpeg);
		output = new unsigned char[output_allocated];
	}
	
	output_size = mjpeg_output_size(mjpeg);
	memcpy(output, mjpeg_output_buffer(mjpeg), output_size);
}


FileMOVThread::FileMOVThread(FileMOV *filemov, int fields) : Thread()
{
	this->filemov = filemov;
	this->fields = fields;
	mjpeg = 0;
}
FileMOVThread::~FileMOVThread()
{
}

int FileMOVThread::start_encoding()
{
//printf("FileMOVThread::start_encoding 1 %d\n", fields);
	mjpeg = mjpeg_new(filemov->asset->width, 
		filemov->asset->height, 
		fields);
	mjpeg_set_quality(mjpeg, filemov->asset->quality);
	mjpeg_set_float(mjpeg, 0);
	done = 0;
	Thread::synchronous = 1;
	input_lock.lock();
	start();
}

int FileMOVThread::stop_encoding()
{
	done = 1;
	input_lock.unlock();
	join();
	if(mjpeg) mjpeg_delete(mjpeg);
return 0;
}

int FileMOVThread::encode_buffer()
{
	input_lock.unlock();
return 0;
}

void FileMOVThread::run()
{
	while(!done)
	{
		input_lock.lock();

		if(!done)
		{
// Get a frame to compress.
			filemov->threadframe_lock.lock();
			if(filemov->current_threadframe < filemov->total_threadframes)
			{
// Frame is available to process.
				input_lock.unlock();
				threadframe = filemov->threadframes.values[filemov->current_threadframe];
				filemov->current_threadframe++;
				filemov->threadframe_lock.unlock();

//printf("FileMOVThread 1 %d\n", threadframe->input->get_color_model());
				mjpeg_compress(mjpeg, 
					threadframe->input->get_rows(), 
					0, 
					0, 
					0,
					BC_RGB888,
					1);

//printf("FileMOVThread 1\n");
				if(fields > 1)
				{
					long field2_offset;
					mjpeg_insert_quicktime_markers(&mjpeg->output_data,
						&mjpeg->output_size,
						&mjpeg->output_allocated,
						fields,
						&field2_offset);
				}
				threadframe->load_output(mjpeg);

//printf("FileMOVThread 2\n");
				threadframe->completion_lock.unlock();
			}
			else
				filemov->threadframe_lock.unlock();
		}
	}
}

