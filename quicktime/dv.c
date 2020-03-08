#include "colormodels.h"
#include "dv.h"
#include "funcprotos.h"
#include "quicktime.h"

static int quicktime_delete_codec_dv(quicktime_video_map_t *vtrack)
{
	quicktime_dv_codec_t *codec = ((quicktime_codec_t*)vtrack->codec)->priv;

	if(codec->dv) dv_delete(codec->dv);
	free(codec->temp_frame);
	free(codec->data);
	free(codec);
	return 0;
}

static int quicktime_decode_dv(quicktime_t *file, unsigned char **row_pointers, int track)
{
	long bytes;
	quicktime_video_map_t *vtrack = &(file->vtracks[track]);
	quicktime_dv_codec_t *codec = ((quicktime_codec_t*)vtrack->codec)->priv;
	int width = vtrack->track->tkhd.track_width;
	int height = vtrack->track->tkhd.track_height;
	int result = 0;

	quicktime_set_video_position(file, vtrack->current_position, track);
	bytes = quicktime_frame_size(file, vtrack->current_position, track);
	result = !quicktime_read_data(file, codec->data, bytes);

	if(codec->dv)
	{
		if(width == 720 && height == 480)
		{
			dv_read_video(codec->dv, row_pointers, codec->data, bytes, file->color_model);
		}
		else
		{
			register int x, y;
			dv_read_video(codec->dv, codec->temp_rows, codec->data, bytes, file->color_model);

			for(y = 0; y < height; y++)
			{
				for(x = 0; x < width * 3; x++)
				{
					row_pointers[y][x] = codec->temp_rows[y][x];
				}
			}
		}
	}

	return result;
}

static int quicktime_encode_dv(quicktime_t *file, unsigned char **row_pointers, int track)
{
	fprintf(stderr, "quicktime_encode_dv: DV encoder not available\n");
	return 1;
}

// Logic: DV contains a mixture of 420 and 411 so can only output 444 or 422

static int quicktime_reads_colormodel_dv(quicktime_t *file, 
		int colormodel, 
		int track)
{
	return (colormodel == BC_RGB888 ||
		colormodel == BC_YUV888);
}

void quicktime_init_codec_dv(quicktime_video_map_t *vtrack)
{
	quicktime_dv_codec_t *codec;
	int i;

/* Init public items */
	((quicktime_codec_t*)vtrack->codec)->priv = calloc(1, sizeof(quicktime_dv_codec_t));
	((quicktime_codec_t*)vtrack->codec)->delete_vcodec = quicktime_delete_codec_dv;
	((quicktime_codec_t*)vtrack->codec)->decode_video = quicktime_decode_dv;
	((quicktime_codec_t*)vtrack->codec)->encode_video = quicktime_encode_dv;
	((quicktime_codec_t*)vtrack->codec)->decode_audio = 0;
	((quicktime_codec_t*)vtrack->codec)->encode_audio = 0;
	((quicktime_codec_t*)vtrack->codec)->reads_colormodel = quicktime_reads_colormodel_dv;

/* Init private items */
	codec = ((quicktime_codec_t*)vtrack->codec)->priv;
	codec->dv = dv_new();
	codec->temp_frame = calloc(1, 720 * 576 * 3);
	codec->data = calloc(1, 140000);
	for(i = 0; i < 576; i++) codec->temp_rows[i] = codec->temp_frame + 720 * 3 * i;
}
