#include "main.h"
/*************************************************************************
	Initialisieren von Strukturen fuer S*N cc Compiler.

	Initialize structs for S*N cc compiler.
*************************************************************************/
void empty_struc (stream)
	
stream_element *stream;
{
    switch (stream->type) {
        case STREAMS_AUDIO:
	    empty_audio_struc(&stream->info.audio);
	    break;
	case STREAMS_VIDEO:
	    empty_video_struc(&stream->info.video);
	    break;
    }
}

void empty_video_struc (pointer)

Video_struc *pointer;
{
    int i;

    pointer->stream_length	= 0;
    pointer->num_sequence 	= 0;
    pointer->num_seq_end	= 0;
    pointer->num_pictures 	= 0;
    pointer->num_groups   	= 0;
    for (i=0; i<4; i++)
    {
        pointer->num_frames[i] 	= 0;
        pointer->avg_frames[i]	= 0;
    }
    pointer->horizontal_size 	= 0;
    pointer->vertical_size	= 0;
    pointer->aspect_ratio 	= 0;
    pointer->picture_rate 	= 0;
    pointer->bit_rate		= 0;
    pointer->comp_bit_rate	= 0;
    pointer->vbv_buffer_size	= 0;
    pointer->CSPF		= 0;
}

void empty_audio_struc (pointer)

Audio_struc *pointer;
{   
    int i;

    pointer->stream_length 	= 0;
    pointer->num_syncword 	= 0;
    for (i=0; i<2; i++)
    {
        pointer->num_frames [i]	= 0;
        pointer->size_frames[i]	= 0;
    }
    pointer->layer		= 0;
    pointer->protection 	= 0;
    pointer->bit_rate 		= 0;
    pointer->frequency 		= 0;
    pointer->mode 		= 0;
    pointer->mode_extension 	= 0;
    pointer->copyright      	= 0;
    pointer->original_copy  	= 0;
    pointer->emphasis		= 0;
}

void empty_aunit_struc (stream)
	
stream_element *stream;
{
    switch (stream->type){
        case STREAMS_AUDIO:
	    empty_aaunit_struc(&stream->au.audio);
	    break;
	case STREAMS_VIDEO:
	    empty_vaunit_struc(&stream->au.video);
	    break;
    }
}

void empty_vaunit_struc (pointer)
Vaunit_struc *pointer;
{
    pointer->length = 0;
    pointer->type   = 0;
    empty_timecode_struc (&pointer->DTS);
    empty_timecode_struc (&pointer->PTS);
}

void empty_aaunit_struc (pointer)
Aaunit_struc *pointer;
{
    pointer->length = 0;
    empty_timecode_struc (&pointer->PTS);
}

void empty_sector_struc (pointer)
Sector_struc *pointer;
{
    pointer->length_of_sector  = 0;
    pointer->length_of_packet_data  = 0;
    empty_timecode_struc (&pointer->TS);
}


void init_buffer_struc (pointer, size)
Buffer_struc *pointer;
unsigned int size;
{
    pointer->max_size = size;
    pointer->first = NULL;
}
