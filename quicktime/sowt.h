#ifndef QUICKTIME_SOWT_H
#define QUICKTIME_SOWT_H

//extern void quicktime_init_codec_sowt(quicktime_audio_map_t *);

#include "quicktime.h"

typedef struct
{
	char *work_buffer;
	long buffer_size;
} quicktime_sowt_codec_t;

#endif
