#include "colormodels.h"
#include "funcprotos.h"
#include "quicktime.h"
#include <glib.h>
#include <sys/stat.h>

static longest get_file_length(quicktime_t *file)
{
	struct stat status;
	if(fstat(fileno(file->stream), &status))
		perror("get_file_length fstat:");
	return status.st_size;
}

int quicktime_make_streamable(char *in_path, char *out_path)
{
	quicktime_t file, *old_file, new_file;
	int moov_exists = 0, mdat_exists = 0, result, atoms = 1;
	longest mdat_start, mdat_size;
	quicktime_atom_t leaf_atom;
	longest moov_length;
	
	quicktime_init(&file);

/* find the moov atom in the old file */
	
	if(!(file.stream = fopen(in_path, "rb")))
	{
		perror("quicktime_make_streamable");
		return 1;
	}

	file.total_length = get_file_length(&file);

/* get the locations of moov and mdat atoms */
	do
	{
/*printf("%x\n", quicktime_position(&file)); */
		result = quicktime_atom_read_header(&file, &leaf_atom);

		if(!result)
		{
			if(quicktime_atom_is(&leaf_atom, "moov"))
			{
				moov_exists = atoms;
				moov_length = leaf_atom.size;
			}
			else
			if(quicktime_atom_is(&leaf_atom, "mdat"))
			{
				mdat_start = quicktime_position(&file) - HEADER_LENGTH;
				mdat_size = leaf_atom.size;
				mdat_exists = atoms;
			}

			quicktime_atom_skip(&file, &leaf_atom);

			atoms++;
		}
	}while(!result && quicktime_position(&file) < file.total_length);

	fclose(file.stream);

	if(!moov_exists)
	{
		printf("quicktime_make_streamable: no moov atom\n");
		return 1;
	}

	if(!mdat_exists)
	{
		printf("quicktime_make_streamable: no mdat atom\n");
		return 1;
	}

/* copy the old file to the new file */
	if(moov_exists && mdat_exists)
	{
/* moov wasn't the first atom */
		if(moov_exists > 1)
		{
			char *buffer;
			longest buf_size = 1000000;

			result = 0;

/* read the header proper */
			if(!(old_file = quicktime_open(in_path, 1, 0)))
			{
				return 1;
			}

			quicktime_shift_offsets(&(old_file->moov), moov_length);

/* open the output file */
			if(!(new_file.stream = fopen(out_path, "wb")))
			{
				perror("quicktime_make_streamable");
				result =  1;
			}
			else
			{
/* set up some flags */
				new_file.wr = 1;
				new_file.rd = 0;
				quicktime_write_moov(&new_file, &(old_file->moov));
				quicktime_set_position(old_file, mdat_start);

				if(!(buffer = calloc(1, buf_size)))
				{
					result = 1;
					printf("quicktime_make_streamable: out of memory\n");
				}
				else
				{
					while(quicktime_position(old_file) < mdat_start + mdat_size && !result)
					{
						if(quicktime_position(old_file) + buf_size > mdat_start + mdat_size)
							buf_size = mdat_start + mdat_size - quicktime_position(old_file);

						if(!quicktime_read_data(old_file, buffer, buf_size)) result = 1;
						if(!result)
						{
							if(!quicktime_write_data(&new_file, buffer, buf_size)) result = 1;
						}
					}
					free(buffer);
				}
				fclose(new_file.stream);
			}
			quicktime_close(old_file);
		}
		else
		{
			printf("quicktime_make_streamable: header already at 0 offset\n");
			return 0;
		}
	}
	
	return 0;
}

void quicktime_set_copyright(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.copyright), &(file->moov.udta.copyright_len), string);
}

void quicktime_set_name(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.name), &(file->moov.udta.name_len), string);
}

void quicktime_set_info(quicktime_t *file, char *string)
{
	quicktime_set_udta_string(&(file->moov.udta.info), &(file->moov.udta.info_len), string);
}

char* quicktime_get_copyright(quicktime_t *file)
{
	return file->moov.udta.copyright;
}

char* quicktime_get_name(quicktime_t *file)
{
	return file->moov.udta.name;
}

char* quicktime_get_info(quicktime_t *file)
{
	return file->moov.udta.info;
}


int quicktime_video_tracks(quicktime_t *file)
{
	int i, result = 0;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		if(file->moov.trak[i]->mdia.minf.is_video) result++;
	}
	return result;
}

int quicktime_audio_tracks(quicktime_t *file)
{
	int i, result = 0;
	quicktime_minf_t *minf;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		minf = &(file->moov.trak[i]->mdia.minf);
		if(minf->is_audio)
			result++;
	}
	return result;
}

int quicktime_set_audio(quicktime_t *file, 
						int channels,
						long sample_rate,
						int bits,
						char *compressor)
{
	quicktime_trak_t *trak;

/* allocate an arbitrary number of tracks */
	if(channels)
	{
/* Fake the bits parameter for some formats. */
		if(quicktime_match_32(compressor, QUICKTIME_ULAW) ||
			quicktime_match_32(compressor, QUICKTIME_IMA4)) bits = 16;

		file->atracks = (quicktime_audio_map_t*)calloc(1, sizeof(quicktime_audio_map_t));
		trak = quicktime_add_track(&(file->moov));
		quicktime_trak_init_audio(file, trak, channels, sample_rate, bits, compressor);
		quicktime_init_audio_map(&(file->atracks[0]), trak);
		file->atracks[file->total_atracks].track = trak;
		file->atracks[file->total_atracks].channels = channels;
		file->atracks[file->total_atracks].current_position = 0;
		file->atracks[file->total_atracks].current_chunk = 1;
		file->total_atracks++;
	}
	return 1;   /* Return the number of tracks created */
}

int quicktime_set_video(quicktime_t *file, 
						int tracks, 
						int frame_w, 
						int frame_h,
						float frame_rate,
						char *compressor)
{
	int i;
	quicktime_trak_t *trak;

	if(tracks)
	{
//printf("quicktime_set_video 1\n");
		quicktime_mhvd_init_video(file, &(file->moov.mvhd), frame_rate);
//printf("quicktime_set_video 1\n");
		file->total_vtracks = tracks;
//printf("quicktime_set_video 1\n");
		file->vtracks = (quicktime_video_map_t*)calloc(1, sizeof(quicktime_video_map_t) * file->total_vtracks);
//printf("quicktime_set_video 1\n");
		for(i = 0; i < tracks; i++)
		{
//printf("quicktime_set_video 2\n");
			trak = quicktime_add_track(&(file->moov));
//printf("quicktime_set_video 2\n");
			quicktime_trak_init_video(file, trak, frame_w, frame_h, frame_rate, compressor);
//printf("quicktime_set_video 3\n");
			quicktime_init_video_map(&(file->vtracks[i]), trak);
//printf("quicktime_set_video 3\n");
		}
//printf("quicktime_set_video 4\n");
	}
//printf("quicktime_set_video 5\n");
	return 0;
}

void quicktime_set_framerate(quicktime_t *file, float framerate)
{
	int i;
	int new_time_scale, new_sample_duration;

	new_time_scale = quicktime_get_timescale(framerate);
	new_sample_duration = (int)((float)new_time_scale / framerate + 0.5);

	for(i = 0; i < file->total_vtracks; i++)
	{
		file->vtracks[i].track->mdia.mdhd.time_scale = new_time_scale;
		file->vtracks[i].track->mdia.minf.stbl.stts.table[0].sample_duration = new_sample_duration;
	}
}

quicktime_trak_t* quicktime_add_track(quicktime_moov_t *moov)
{
	quicktime_trak_t *trak;
	trak = moov->trak[moov->total_tracks] = calloc(1, sizeof(quicktime_trak_t));
	quicktime_trak_init(trak);
	trak->tkhd.track_id = moov->mvhd.next_track_id;
	moov->mvhd.next_track_id++;
	moov->total_tracks++;
	return trak;
}

/* ============================= Initialization functions */

int quicktime_init(quicktime_t *file)
{
	bzero(file, sizeof(quicktime_t));
//	quicktime_atom_write_header64(new_file, &file->mdat.atom, "mdat");
	quicktime_moov_init(&(file->moov));
	file->cpus = 1;
	file->color_model = BC_RGB888;
	return 0;
}

int quicktime_delete(quicktime_t *file)
{
	int i;
//printf("quicktime_delete 1\n");
	if(file->total_atracks) 
	{
		for(i = 0; i < file->total_atracks; i++)
			quicktime_delete_audio_map(&(file->atracks[i]));
		free(file->atracks);
	}
//printf("quicktime_delete 1\n");
	if(file->total_vtracks)
	{
		for(i = 0; i < file->total_vtracks; i++)
			quicktime_delete_video_map(&(file->vtracks[i]));
		free(file->vtracks);
	}
//printf("quicktime_delete 1\n");
	file->total_atracks = 0;
	file->total_vtracks = 0;
//printf("quicktime_delete 1\n");
	if(file->preload_size)
	{
		free(file->preload_buffer);
		file->preload_size = 0;
	}
//printf("quicktime_delete 1\n");
	quicktime_moov_delete(&(file->moov));
//printf("quicktime_delete 1\n");
	quicktime_mdat_delete(&(file->mdat));
//printf("quicktime_delete 2\n");
	return 0;
}

/* =============================== Optimization functions */

int quicktime_set_cpus(quicktime_t *file, int cpus)
{
	if(cpus > 0) file->cpus = cpus;
	return 0;
}

void quicktime_set_preload(quicktime_t *file, longest preload)
{
	if(preload)
		if(!file->preload_size)
		{
			file->preload_size = preload;
			file->preload_buffer = calloc(1, preload);
			file->preload_start = 0;
			file->preload_end = 0;
			file->preload_ptr = 0;
		}
}


int quicktime_get_timescale(float frame_rate)
{
	int timescale = 600;
/* Encode the 29.97, 23.976, 59.94 framerates */
	if(frame_rate - (int)frame_rate != 0) 
		timescale = (int)(frame_rate * 1001 + 0.5);
	else
	if((600 / frame_rate) - (int)(600 / frame_rate) != 0) 
			timescale = (int)(frame_rate * 100 + 0.5);
//printf("quicktime_get_timescale %f %d\n", 600 / frame_rate, (int)(600 / frame_rate));
	return timescale;
}

int quicktime_seek_end(quicktime_t *file)
{
	quicktime_set_position(file, file->mdat.atom.size + file->mdat.atom.start + HEADER_LENGTH * 2);
/*printf("quicktime_seek_end %ld\n", file->mdat.atom.size + file->mdat.atom.start); */
	quicktime_update_positions(file);
	return 0;
}

int quicktime_seek_start(quicktime_t *file)
{
	quicktime_set_position(file, file->mdat.atom.start + HEADER_LENGTH * 2);
	quicktime_update_positions(file);
	return 0;
}

long quicktime_audio_length(quicktime_t *file, int track)
{
	if(file->total_atracks > 0) 
		return quicktime_track_samples(file, file->atracks[track].track);

	return 0;
}

long quicktime_video_length(quicktime_t *file, int track)
{
/*printf("quicktime_video_length %d %d\n", quicktime_track_samples(file, file->vtracks[track].track), track); */
	if(file->total_vtracks > 0)
		return quicktime_track_samples(file, file->vtracks[track].track);
	return 0;
}

long quicktime_audio_position(quicktime_t *file, int track)
{
	return file->atracks[track].current_position;
}

long quicktime_video_position(quicktime_t *file, int track)
{
	return file->vtracks[track].current_position;
}

int quicktime_update_positions(quicktime_t *file)
{
/* Get the sample position from the file offset */
/* for routines that change the positions of all tracks, like */
/* seek_end and seek_start but not for routines that reposition one track, like */
/* set_audio_position. */

	longest mdat_offset = quicktime_position(file) - file->mdat.atom.start;
	longest sample, chunk, chunk_offset;
	int i;

	if(file->total_atracks)
	{
		sample = quicktime_offset_to_sample(file->atracks[0].track, mdat_offset);
		chunk = quicktime_offset_to_chunk(&chunk_offset, file->atracks[0].track, mdat_offset);
		for(i = 0; i < file->total_atracks; i++)
		{
			file->atracks[i].current_position = sample;
			file->atracks[i].current_chunk = chunk;
		}
	}

	if(file->total_vtracks)
	{
		sample = quicktime_offset_to_sample(file->vtracks[0].track, mdat_offset);
		chunk = quicktime_offset_to_chunk(&chunk_offset, file->vtracks[0].track, mdat_offset);
		for(i = 0; i < file->total_vtracks; i++)
		{
			file->vtracks[i].current_position = sample;
			file->vtracks[i].current_chunk = chunk;
		}
	}
	return 0;
}

int quicktime_set_audio_position(quicktime_t *file, longest sample, int track)
{
	longest offset, chunk_sample, chunk;
	quicktime_trak_t *trak;

	if(file->total_atracks)
	{
		trak = file->atracks[track].track;
		file->atracks[track].current_position = sample;
		quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, sample);
		file->atracks[track].current_chunk = chunk;
		offset = quicktime_sample_to_offset(trak, sample);
		quicktime_set_position(file, offset);
	}

	return 0;
}

int quicktime_set_video_position(quicktime_t *file, longest frame, int track)
{
	longest offset, chunk_sample, chunk;
	quicktime_trak_t *trak;

	if(file->total_vtracks)
	{
		trak = file->vtracks[track].track;
		file->vtracks[track].current_position = frame;
		quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, frame);
		file->vtracks[track].current_chunk = chunk;
		offset = quicktime_sample_to_offset(trak, frame);
		quicktime_set_position(file, offset);
	}
	return 0;
}

int quicktime_has_audio(quicktime_t *file)
{
	if(quicktime_audio_tracks(file)) return 1;
	return 0;
}

long quicktime_sample_rate(quicktime_t *file, int track)
{
	if(file->total_atracks)
		return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_rate;
	return 0;
}

int quicktime_audio_bits(quicktime_t *file, int track)
{
	if(file->total_atracks)
		return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].sample_size;

	return 0;
}

char* quicktime_audio_compressor(quicktime_t *file, int track)
{
	return file->atracks[track].track->mdia.minf.stbl.stsd.table[0].format;
}

int quicktime_track_channels(quicktime_t *file, int track)
{
	if(track < file->total_atracks)
		return file->atracks[track].channels;

	return 0;
}

int quicktime_channel_location(quicktime_t *file, int *quicktime_track, int *quicktime_channel, int channel)
{
	int current_channel = 0, current_track = 0;
	*quicktime_channel = 0;
	*quicktime_track = 0;
	for(current_channel = 0, current_track = 0; current_track < file->total_atracks; )
	{
		if(channel >= current_channel)
		{
			*quicktime_channel = channel - current_channel;
			*quicktime_track = current_track;
		}

		current_channel += file->atracks[current_track].channels;
		current_track++;
	}
	return 0;
}

int quicktime_has_video(quicktime_t *file)
{
	if(quicktime_video_tracks(file)) return 1;
	return 0;
}

int quicktime_video_width(quicktime_t *file, int track)
{
	if(file->total_vtracks)
		return file->vtracks[track].track->tkhd.track_width;
	return 0;
}

int quicktime_video_height(quicktime_t *file, int track)
{
	if(file->total_vtracks)
		return file->vtracks[track].track->tkhd.track_height;
	return 0;
}

int quicktime_video_depth(quicktime_t *file, int track)
{
	if(file->total_vtracks)
		return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].depth;
	return 0;
}

void quicktime_set_cmodel(quicktime_t *file, int colormodel)
{
	file->color_model = colormodel;
}

void quicktime_set_depth(quicktime_t *file, int depth, int track)
{
	int i;

	for(i = 0; i < file->total_vtracks; i++)
	{
		file->vtracks[i].track->mdia.minf.stbl.stsd.table[0].depth = depth;
	}
}

float quicktime_frame_rate(quicktime_t *file, int track)
{
	if(file->total_vtracks > track)
		return (float)file->vtracks[track].track->mdia.mdhd.time_scale / 
			file->vtracks[track].track->mdia.minf.stbl.stts.table[0].sample_duration;

	return 0;
}

char* quicktime_video_compressor(quicktime_t *file, int track)
{
	return file->vtracks[track].track->mdia.minf.stbl.stsd.table[0].format;
}

int quicktime_write_audio(quicktime_t *file, char *audio_buffer, long samples, int track)
{
	longest offset;
	int result;
	longest bytes;

/* write chunk for 1 track */
	bytes = samples * quicktime_audio_bits(file, track) / 8 * file->atracks[track].channels;
	offset = quicktime_position(file);
	result = quicktime_write_data(file, audio_buffer, bytes);

	if(result) result = 0; else result = 1; /* defeat fwrite's return */
	quicktime_update_tables(file, 
						file->atracks[track].track, 
						offset, 
						file->atracks[track].current_chunk, 
						file->atracks[track].current_position, 
						samples, 
						0);
	file->atracks[track].current_position += samples;
	file->atracks[track].current_chunk++;
	return result;
}

int quicktime_write_frame(quicktime_t *file, unsigned char *video_buffer, longest bytes, int track)
{
	longest offset = quicktime_position(file);
	int result = 0;

	result = !quicktime_write_data(file, video_buffer, bytes);

	quicktime_update_tables(file,
						file->vtracks[track].track,
						offset,
						file->vtracks[track].current_chunk,
						file->vtracks[track].current_position,
						1,
						bytes);
	file->vtracks[track].current_position++;
	file->vtracks[track].current_chunk++;
	return result;
}


long quicktime_read_audio(quicktime_t *file, char *audio_buffer, long samples, int track)
{
	longest chunk_sample, chunk;
	int result = 1, track_num;
	quicktime_trak_t *trak = file->atracks[track].track;
	longest fragment_len, chunk_end;
	longest position = file->atracks[track].current_position;
	longest start = position, end = position + samples;
	longest bytes, total_bytes = 0;
	longest buffer_offset;

	quicktime_chunk_of_sample(&chunk_sample, &chunk, trak, position);
	buffer_offset = 0;

	while(position < end && result)
	{
		quicktime_set_audio_position(file, position, track);
		fragment_len = quicktime_chunk_samples(trak, chunk);
		chunk_end = chunk_sample + fragment_len;
		fragment_len -= position - chunk_sample;
		if(position + fragment_len > chunk_end) fragment_len = chunk_end - position;
		if(position + fragment_len > end) fragment_len = end - position;

		bytes = quicktime_samples_to_bytes(trak, fragment_len);
		result = quicktime_read_data(file, &audio_buffer[buffer_offset], bytes);

		total_bytes += bytes;
		position += fragment_len;
		chunk_sample = position;
		buffer_offset += bytes;
		chunk++;
	}

	file->atracks[track].current_position = position;
	if(!result) return 0;
	return total_bytes;
}

int quicktime_read_chunk(quicktime_t *file, char *output, int track, longest chunk, longest byte_start, longest byte_len)
{
	quicktime_set_position(file, quicktime_chunk_to_offset(file->atracks[track].track, chunk) + byte_start);
	if(quicktime_read_data(file, output, byte_len)) return 0;
	else
	return 1;
}

long quicktime_frame_size(quicktime_t *file, long frame, int track)
{
	long bytes = 0;
	quicktime_trak_t *trak = file->vtracks[track].track;

	if(trak->mdia.minf.stbl.stsz.sample_size)
	{
		bytes = trak->mdia.minf.stbl.stsz.sample_size;
	}
	else
	{
		long total_frames = quicktime_track_samples(file, trak);
		if(frame < 0) frame = 0;
		else
		if(frame > total_frames - 1) frame = total_frames - 1;
		bytes = trak->mdia.minf.stbl.stsz.table[frame].size;
	}


	return bytes;
}


long quicktime_read_frame(quicktime_t *file, unsigned char *video_buffer, int track)
{
	longest bytes;
	int result = 0;

	quicktime_trak_t *trak = file->vtracks[track].track;
	bytes = quicktime_frame_size(file, file->vtracks[track].current_position, track);

	quicktime_set_video_position(file, file->vtracks[track].current_position, track);
	result = quicktime_read_data(file, video_buffer, bytes);
	file->vtracks[track].current_position++;

	if(!result) return 0;
	return bytes;
}

int quicktime_read_frame_init(quicktime_t *file, int track)
{
	quicktime_trak_t *trak = file->vtracks[track].track;
	quicktime_set_video_position(file, file->vtracks[track].current_position, track);
	if(quicktime_ftell(file) != file->file_position) 
	{
		FSEEK(file->stream, file->file_position, SEEK_SET);
		file->ftell_position = file->file_position;
	}
	return 0;
}

int quicktime_read_frame_end(quicktime_t *file, int track)
{
	file->file_position = quicktime_ftell(file);
	file->vtracks[track].current_position++;
	return 0;
}

int quicktime_init_video_map(quicktime_video_map_t *vtrack, quicktime_trak_t *trak)
{
	vtrack->track = trak;
	vtrack->current_position = 0;
	vtrack->current_chunk = 1;
	quicktime_init_vcodec(vtrack);
	return 0;
}

int quicktime_delete_video_map(quicktime_video_map_t *vtrack)
{
	int i;
	quicktime_delete_vcodec(vtrack);
	return 0;
}

int quicktime_init_audio_map(quicktime_audio_map_t *atrack, quicktime_trak_t *trak)
{
	atrack->track = trak;
	atrack->channels = trak->mdia.minf.stbl.stsd.table[0].channels;
	atrack->current_position = 0;
	atrack->current_chunk = 1;
	quicktime_init_acodec(atrack);
	return 0;
}

int quicktime_delete_audio_map(quicktime_audio_map_t *atrack)
{
	int i;
	quicktime_delete_acodec(atrack);
	return 0;
}

int quicktime_read_info(quicktime_t *file)
{
	int result = 0, found_moov = 0;
	int i, channel, trak_channel, track;
	longest start_position = quicktime_position(file);
	quicktime_atom_t leaf_atom;
	quicktime_trak_t *trak;
	char avi_test[4];

// Check for Microsoft AVI
	quicktime_read_char32(file, avi_test);
	if(quicktime_match_32(avi_test, "RIFF"))
	{
		quicktime_read_char32(file, avi_test);
		quicktime_read_char32(file, avi_test);
		if(quicktime_match_32(avi_test, "AVI "))
			file->use_avi = 1;
	}

	quicktime_set_position(file, 0LL);

	do
	{
		result = quicktime_atom_read_header(file, &leaf_atom);

		if(!result)
		{
			if(quicktime_atom_is(&leaf_atom, "mdat")) 
			{
				quicktime_read_mdat(file, &(file->mdat), &leaf_atom);
			}
			else
			if(quicktime_atom_is(&leaf_atom, "moov")) 
			{
				quicktime_read_moov(file, &(file->moov), &leaf_atom);
				found_moov = 1;
			}
			else
				quicktime_atom_skip(file, &leaf_atom);
		}
//printf("quicktime_read_info ftell %llx length %llx\n", quicktime_position(file), file->total_length);
	}while(!result && quicktime_position(file) < file->total_length);

/* go back to the original position */
	quicktime_set_position(file, start_position);

	if(found_moov)
	{
/* get tables for all the different tracks */
		file->total_atracks = quicktime_audio_tracks(file);
		file->atracks = (quicktime_audio_map_t*)calloc(1, sizeof(quicktime_audio_map_t) * file->total_atracks);

		for(i = 0, track = 0; i < file->total_atracks; i++)
		{
			while(!file->moov.trak[track]->mdia.minf.is_audio)
				track++;
			quicktime_init_audio_map(&(file->atracks[i]), file->moov.trak[track]);
		}

		file->total_vtracks = quicktime_video_tracks(file);
		file->vtracks = (quicktime_video_map_t*)calloc(1, sizeof(quicktime_video_map_t) * file->total_vtracks);

		for(track = 0, i = 0; i < file->total_vtracks; i++)
		{
			while(!file->moov.trak[track]->mdia.minf.is_video)
				track++;

			quicktime_init_video_map(&(file->vtracks[i]), file->moov.trak[track]);
		}
	}

	return !found_moov;
}


int quicktime_dump(quicktime_t *file)
{
	printf("quicktime_dump\n");
	printf("movie data\n");
	printf(" size %ld\n", file->mdat.atom.size);
	printf(" start %ld\n", file->mdat.atom.start);
	quicktime_moov_dump(&(file->moov));
	return 0;
}


// ================================== Entry points =============================

int quicktime_check_sig(char *path)
{
	quicktime_t file;
	quicktime_atom_t leaf_atom;
	int result1 = 0, result2 = 0;

	quicktime_init(&file);
	if(!(file.stream = fopen(path, "rb"))) 
	{
		perror("quicktime_check_sig");
		return 0;
	}

	file.total_length = get_file_length(&file);

	do
	{
		result1 = quicktime_atom_read_header(&file, &leaf_atom);

		if(!result1)
		{
/* just want the "moov" atom */
			if(quicktime_atom_is(&leaf_atom, "moov"))
			{
				result2 = 1;
			}
			else
				quicktime_atom_skip(&file, &leaf_atom);
		}
	}while(!result1 && !result2 && quicktime_position(&file) < file.total_length);

	fclose(file.stream);

	quicktime_delete(&file);
	return result2;
}

quicktime_t* quicktime_open(char *filename, int rd, int wr)
{
	quicktime_t *new_file = calloc(1, sizeof(quicktime_t));
	char flags[10];
	int exists = 0;

//printf("quicktime_open 1\n");
	quicktime_init(new_file);
	new_file->wr = wr;
	new_file->rd = rd;
	new_file->mdat.atom.start = 0;

//printf("quicktime_open 1\n");
	if(rd && (new_file->stream = fopen(filename, "rb")))
	{
		exists = 1; 
		fclose(new_file->stream); 
	}

//printf("quicktime_open 1\n");


	if(rd && !wr) sprintf(flags, "rb");
	else
	if(!rd && wr) sprintf(flags, "wb");
	else
	if(rd && wr)
	{
		if(exists) 
			sprintf(flags, "rb+");
		else
			sprintf(flags, "wb+");
	}

//printf("quicktime_open 1\n");
	if(!(new_file->stream = fopen(filename, flags)))
	{
		perror("quicktime_open");
		free(new_file);
		return 0;
	}


//printf("quicktime_open 1\n");
	if(rd && exists)
	{
// Get length.
		new_file->total_length = get_file_length(new_file);

		if(quicktime_read_info(new_file))
		{
			quicktime_close(new_file);
			fprintf(stderr, "quicktime_open: error in header\n");
			new_file = 0;
		}
	}

//printf("quicktime_open 1 %d %d\n", wr, exists);
	if(wr)
		if(!exists)
		{
/* start the data atom */
/* also don't want to do this if making a streamable file */
			quicktime_atom_write_header64(new_file, &new_file->mdat.atom, "mdat");
		}
//printf("quicktime_open 2 %llx %llx\n", new_file->ftell_position, new_file->file_position);
	return new_file;
}

int quicktime_close(quicktime_t *file)
{
	int result;
	if(file->wr)
	{
// Defeat errors during the fseek that occurs before every fwrite
		file->total_length = get_file_length(file) + 0x7fffffff;
// Atoms are only written here
		quicktime_write_moov(file, &(file->moov));
		quicktime_atom_write_footer(file, &file->mdat.atom);
	}
	result = fclose(file->stream);

	quicktime_delete(file);
	free(file);
	return result;
}
