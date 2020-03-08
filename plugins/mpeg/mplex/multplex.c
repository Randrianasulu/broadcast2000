#include "main.h"

#ifdef TIMER
    extern long total_sec;
    extern long total_usec;
    extern long global_sec;
    extern long global_usec;
    extern struct timeval  tp_start;
    extern struct timeval  tp_end;
    extern struct timeval  tp_global_start;
    extern struct timeval  tp_global_end;
#endif
unsigned long aunit_struc_length_total(stream_element *streams)
{
    unsigned long length = 0;

    for (; streams; streams = streams->next){
	switch (streams->type){
		case STREAMS_AUDIO:
		    length += streams->au.audio.length;
		    break;
		case STREAMS_VIDEO:
		    length += streams->au.video.length;
		    break;
	}
    }
    return length;
}

/******************************************************************
	Hauptschleife Multiplexroutinenaufruf
	Kuemmert sich um oeffnen und schliessen alles beteiligten
	Dateien und um den korrekten Aufruf der jeweils
	noetigen Video- und Audio-Packet Routinen.
	Gewissermassen passiert hier das Wesentliche des 
	Multiplexens. Die Bufferkapazitaeten und die TimeStamps
	werden ueberprueft und damit entschieden, ob ein Video-
	Audio- oder Padding-Packet erstellt und geschrieben
	werden soll.

	Main multiplex iteration.
	Opens and closes all needed files and manages the correct
	call od the respective Video- and Audio- packet routines.
	The basic multiplexing is done here. Buffer capacity and 
	Timestamp checking is also done here, decision is taken
	wether we should genereate a Video-, Audio- or Padding-
	packet.
******************************************************************/

void outputstream (streams, audio_streams, multi_file)

stream_element *streams;
stream_element *audio_streams;
char 		*multi_file;

{
    stream_element *stream;
    stream_element *audio_str;
    stream_element *video_str;
    FILE *ostream;			/* Outputstream MPEG	*/
    FILE *debug_file;

    unsigned int data_rate=0;		/* AudioVideo Byterate	*/
    unsigned int video_rate=0;
    unsigned int audio_rate=0;
    double delay,audio_delay,video_delay;
    double clock_cycles;
    double audio_next_clock_cycles;
    double video_next_clock_cycles;
    unsigned int bytes_output;
    double dmux_rate;
    unsigned long sectors_delay,video_delay_ms,audio_delay_ms;
    unsigned int mux_rate;
    unsigned char picture_start;
    unsigned char audio_frame_start;
    unsigned int bytes_left;
    unsigned int audio_bytes;
    unsigned int video_bytes;

    unsigned int nsec_a=0;
    unsigned int nsec_v=0;
    unsigned int nsec_p=0;

    unsigned char* index;
	
    Timecode_struc SCR_audio_delay;
    Timecode_struc SCR_video_delay;
    Timecode_struc current_SCR;
    Timecode_struc audio_next_SCR;
    Timecode_struc video_next_SCR;

    Buffer_struc video_buffer;
    Buffer_struc audio_buffer;

    Pack_struc 		pack;
    Sys_header_struc 	sys_header;
    Sector_struc 	sector;
    Timecode_struc	timestamp;

    unsigned long sector_size;
    unsigned long min_packet_data;
    unsigned long max_packet_data;
    unsigned long packets_per_pack;
    unsigned long audio_buffer_size;
    unsigned long video_buffer_size;

    unsigned long write_pack;
    unsigned char marker_pack;
    unsigned long packet_data_size;
    unsigned char verbose;

    /* Oeffne alle Ein- und Ausgabefiles			*/
    /* Open in- and outputstream				*/

    for (stream = streams; stream; stream=stream->next){
	    stream->file = fopen (stream->file_name, "rb");
	    stream->units = fopen (stream->units_name, "rb");
	    stream->start = FALSE;
	    empty_aunit_struc (stream);
    }

    ostream	= fopen (multi_file, "wb");
    debug_file = fopen ("debug-my", "w");

    /* Einlesen erster Access Unit Informationen		*/
    /* read in first access unit information			*/
    for (stream = streams; stream; stream = stream->next){
	switch (stream->type){
		case STREAMS_AUDIO:
		    fread (&stream->au.audio, sizeof(Aaunit_struc), 1,
			    stream->units);
		    break;
		case STREAMS_VIDEO:
		    fread (&stream->au.video, sizeof(Vaunit_struc), 1,
			    stream->units);
		    break;
	}
	stream->start = TRUE;
    }


printf("\nMerging elementary streams to MPEG/SYSTEMS multiplexed stream.\n");
printf("\n+------------------ MPEG/SYSTEMS INFORMATION -----------------+\n");
    
//    do 
//    {
//    	printf ("\nsector size (CD-ROM 2324 bytes)          : ");
//    	scanf ("%ld", &sector_size);
sector_size = 2048;
//    } while (sector_size>MAX_SECTOR_SIZE);

//    printf   ("packs to packets ratio                 1 : ");
//    scanf ("%ld", &packets_per_pack);
packets_per_pack = 1;
//    printf ("\nSTD video buffer in kB (CSPS: max 46 kB) : ");
//    scanf ("%ld", &video_buffer_size);
video_buffer_size = 46;
//    printf   ("STD audio buffer in kB (CSPS: max  4 kB) : ");
//    scanf ("%ld", &audio_buffer_size);
audio_buffer_size = 4;

    write_pack = packets_per_pack;
    video_buffer_size *= 1024;
    audio_buffer_size *= 1024;
    min_packet_data = sector_size - PACK_HEADER_SIZE - SYS_HEADER_SIZE -
		PACKET_HEADER_SIZE - AFTER_PACKET_LENGTH;
    max_packet_data = sector_size - PACKET_HEADER_SIZE - AFTER_PACKET_LENGTH;

    /* FIXME: if we have only one stream, we have 3 more bytes in the sys header free */
    sys_header.size = SYS_HEADER_SIZE - 6;
    min_packet_data += 6;
    for (stream = streams; stream; stream=stream->next){
 	min_packet_data -= 3; 
	sys_header.size += 3;
    }
    fprintf(debug_file, "min_packet_data: %d\n", min_packet_data);

    /* Bufferstrukturen Initialisieren				*/
    /* initialize buffer structure				*/

    for (stream = streams; stream; stream = stream->next){
	unsigned long size;
	switch (stream->type){
		case STREAMS_AUDIO: size = audio_buffer_size; break;
		case STREAMS_VIDEO: size = video_buffer_size; break;
	}
	init_buffer_struc (&stream->buffer, size);
    }


    /*	DTS ist in den ersten Units i.d.R. gleich null. Damit
	kein Bufferunterlauf passiert, muss berechnet werden, 
	wie lange es dauert, bis alle Daten sowohl des ersten
	Video- als auch des ersten Audio-Access units ankommen.
	Diesen Wert dann als Art Startup-Delay zu den Time-
	stamps dazurechnen. Um etwas Spielraum zu haben, wird
	als Wert einfach die Anzahl der zu uebertragenden
	Sektoren aufgerundet.					*/
    /*  DTS of the first units is supposed to be zero. To avoid
	Buffer underflow, we have to compute how long it takes for
	all first Video and Audio access units to arrive at the 
	system standard decoder buffer. This delay is added as a 
	kind of startup delay to all of the TimeStamps. We compute
	a ceiling based on the number of sectors we will have
	to transport for the first access units */

    for (stream = streams; stream; stream = stream->next){
	switch (stream->type){
		case STREAMS_VIDEO:
		    if (stream->info.video.bit_rate 
				    > stream->info.video.comp_bit_rate)
			stream->rate = stream->info.video.bit_rate * 50;
		    else
			stream->rate = stream->info.video.comp_bit_rate * 50;
		    break;
		case STREAMS_AUDIO:
			stream->rate = bitrate_index
				[3-stream->info.audio.layer]
				[stream->info.audio.bit_rate]*128;
		    break;
	}
	data_rate += stream->rate;
    }

    dmux_rate =  ceil((double)(data_rate) *
		 ((double)(sector_size)/(double)(min_packet_data) +
		 ((double)(sector_size)/(double)(max_packet_data) *
		 (double)(packets_per_pack-1.))) / (double)(packets_per_pack) );
    data_rate = ceil(dmux_rate/50.)*50;

    printf ("\ncomputed multiplexed stream data rate    : %7.3f\n",dmux_rate);
    printf ("target data rate                           : %6u\n",data_rate);
dmux_rate = data_rate;
//    printf ("\nstartup sectors_delay                    : ");
//    scanf  ("%u", &sectors_delay);
sectors_delay = 0;
//    printf ("video stream startup offset (ms)         : ");
//    scanf  ("%u", &video_delay_ms);
video_delay_ms = 0;
//    printf ("audio stream startup offset (ms)         : ");
//    scanf  ("%u", &audio_delay_ms);
audio_delay_ms = 0;

    video_delay = (double)video_delay_ms*(double)(CLOCKS/1000);
    audio_delay = (double)audio_delay_ms*(double)(CLOCKS/1000);

    verbose=ask_verbose();
    printf ("\n");

#ifdef TIMER
    gettimeofday (&tp_global_start,NULL);
#endif

    mux_rate = ceil(dmux_rate/50.);
    dmux_rate= mux_rate * 50.;

    delay = (double)sectors_delay;
    FOR_EACH_STR(stream, streams, /*FIXME*/
	,
	delay += ceil((double)stream->au.video.length/(double)min_packet_data);,
	delay += ceil((double)stream->au.audio.length/(double)min_packet_data);
    );
    delay *= (double)sector_size/dmux_rate*(double)CLOCKS;

    audio_delay += delay;
    video_delay += delay;

    make_timecode (audio_delay, &SCR_audio_delay);
    make_timecode (video_delay, &SCR_video_delay);


    for (stream = streams; stream; stream = stream->next){
	switch (stream->type){
		case STREAMS_AUDIO:
    		    add_to_timecode (&SCR_audio_delay, &stream->au.audio.PTS);
		    break;
		case STREAMS_VIDEO:
		    add_to_timecode (&SCR_video_delay, &stream->au.video.DTS);
		    add_to_timecode (&SCR_video_delay, &stream->au.video.PTS);
		    break;
	}
    }

    bytes_output = 0;

    /* 	Jetzt probieren wir mal, Unit fuer Unit auszulesen und 
	ins Outputstream auszuschreiben. Die Schwierigkeit liegt
	darin, dass die Buffer konstant ueberprueft werden muessen
	und dass die jeweilige Access Unit auch innerhalb des DTS
	eintreten muss. Es kann passieren, dass z.B. im Video-
	Buffer noch ein altes Picture liegt, das noch dekodiert
	werden muss, wir jetzt schon das naechste schicken, aber
	nach einigen Packets der Buffer voll ist. Da darf nichts
	mehr geschickt werden, bis die DTS des alten Bildes
	eingetreten ist und damit der Buffer kleiner geworden ist.
	In der Zwischenzeit kann ein Audio-Packet geschickt werden
	und/oder ein Padding-Packet generiert werden.		*/
    /*  Let's try to read in unit after unit and to write it out into
	the outputstream. The only difficulty herein lies into the 
	buffer management, and into the fact the the actual access
	unit *has* to arrive in time, that means the whole unit
	(better yet, packet data), has to arrive before arrival of
	DTS. If both buffers are full we'll generate a padding packet */


    status_header ();


    while (aunit_struc_length_total(streams) > 0)
    {

	if (write_pack-- == packets_per_pack) 
	{
	    marker_pack = TRUE;
	    packet_data_size = min_packet_data;
	} else 
	{
	    marker_pack = FALSE;
	    packet_data_size = max_packet_data;
	}

	if (write_pack == 0) write_pack = packets_per_pack;

	clock_cycles = (double)(bytes_output+LAST_SCR_BYTE_IN_PACK)*
		CLOCKS/dmux_rate;
	make_timecode (clock_cycles, &current_SCR);

        for (stream = streams; stream; stream = stream->next){
	    unsigned long length = 0;
	    switch (stream->type){
		    case STREAMS_AUDIO: length = stream->au.audio.length; break;
		    case STREAMS_VIDEO: length = stream->au.video.length; break;
	    }
	    stream->bytes = (length/min_packet_data) * sector_size
				+ (length%min_packet_data)
				+ (sector_size-min_packet_data);
	    stream->next_clock_cycles = (double)(bytes_output+sector_size
					+ stream->bytes)/dmux_rate*CLOCKS;
	    make_timecode (stream->next_clock_cycles, &stream->next_SCR);
	    buffer_clean (&stream->buffer, &current_SCR);
	}


	/* FALL: Video Buffer OK, Video Daten vorhanden		*/
	/*       Audio Daten werden on time ankommen		*/
	/* CASE: Video Buffer OK, Video Data ready		*/
	/*	 Audio Data will arrive on time			*/
	FIND_VIDEO_STR(video_str, streams,
			(buffer_space(&video_str->buffer) >= packet_data_size)
			    && (video_str->au.video.length > 0));
	FIND_AUDIO_STR(audio_str, streams,
		!comp_timecode(&audio_str->next_SCR, &audio_str->au.audio.PTS)
		&& !(audio_str->au.audio.length==0));

	if (video_str && !audio_str)
	{
	    fputs("CASE:1\n", debug_file);
	    /* video packet schicken */
	    /* write out video packet */
	    output_video (&current_SCR, &SCR_video_delay, video_str->units,
		video_str->file, ostream, &pack, &sys_header, &sector,
		&video_str->buffer, &video_str->au.video, &video_str->start,
		&bytes_output, mux_rate, audio_buffer_size, video_buffer_size,
		packet_data_size, marker_pack, video_str->nstream, streams);

	    /* status info */
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
#if 0
	    status_info (nsec_a, ++nsec_v, nsec_p, bytes_output,
			 buffer_space(&video_buffer),
			 buffer_space(&audio_buffer),verbose);
#endif
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    continue;
	}

	/* FALL: Audio Buffer OK, Audio Daten vorhanden		*/
	/*       Video Daten werden on time ankommen		*/
	/* CASE: Audio Buffer OK, Audio Data ready		*/
	/*	 Video Data will arrive on time			*/

	FIND_AUDIO_STR(audio_str, streams,
		(buffer_space(&audio_str->buffer) >= packet_data_size)
			    && (audio_str->au.audio.length > 0));
	FIND_VIDEO_STR(video_str, streams,
		!comp_timecode(&video_str->next_SCR, &video_str->au.video.DTS)
		&& !(video_str->au.video.length==0));
			
	if ( audio_str && !video_str)
	{
	    fputs("CASE:2\n", debug_file);
	    /* audio packet schicken */
	    /* write out audio packet */

	    output_audio (&current_SCR, &SCR_audio_delay, audio_str->units,
		audio_str->file, ostream, &pack, &sys_header, &sector,
		&audio_str->buffer, &audio_str->au.audio, &audio_str->start,
		&bytes_output, mux_rate, audio_buffer_size, video_buffer_size,
		packet_data_size, marker_pack,audio_str->nstream, streams);

#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    /* status info */
#if 0
	    status_info (++nsec_a, nsec_v, nsec_p, bytes_output, 
			 buffer_space(&video_buffer),
			 buffer_space(&audio_buffer),verbose);
#endif
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    continue;
	}


	/* FALL: Audio Buffer OK, Audio Daten vorhanden		*/
	/*       Audio Daten werden nicht on time ankommen	*/
	/* CASE: Audio Buffer OK, Audio data ready		*/
	/*	 Audio data will be time out			*/

	FIND_AUDIO_STR(audio_str, streams,
		(buffer_space(&audio_str->buffer) >= packet_data_size)
		    && (audio_str->au.audio.length > 0)
		    &! comp_timecode (&audio_str->next_SCR,
			    &audio_str->au.audio.PTS));

	if ( audio_str )
	{
	    fputs("CASE:3\n", debug_file);
	    /* audio packet schicken */
	    /* write out audio packet */
	    output_audio (&current_SCR, &SCR_audio_delay, audio_str->units,
		audio_str->file, ostream, &pack, &sys_header, &sector,
		&audio_str->buffer, &audio_str->au.audio, &audio_str->start,
		&bytes_output, mux_rate, audio_buffer_size, video_buffer_size,
		packet_data_size, marker_pack, audio_str->nstream, streams);

	    /* audio fehlermeldung */
	    /* audio error message */
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    status_message (STATUS_AUDIO_TIME_OUT);
#if 0
	    /* status info */
	    status_info (++nsec_a, nsec_v, nsec_p, bytes_output, 
			 buffer_space(&video_buffer),
			 buffer_space(&audio_buffer),verbose);
#endif
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    continue;
	}

	/* FALL: Video Buffer OK, Video Daten vorhanden		*/
	/*       Video Daten werden nicht on time ankommen	*/
	/* CASE: Video Buffer OK, Video data ready		*/
	/*	 Video data will be time out			*/
	FIND_VIDEO_STR(video_str, streams,
		(buffer_space(&video_str->buffer) >= packet_data_size)
			    && (video_str->au.video.length > 0)
			    &! comp_timecode (&video_str->next_SCR,
				    &video_str->au.video.DTS));

	if (video_str)
	{
	    fputs("CASE:4\n", debug_file);
	    /* video packet schicken */
	    /* write out video packet */
	    output_video (&current_SCR, &SCR_video_delay, video_str->units,
		video_str->file, ostream, &pack, &sys_header, &sector,
		&video_str->buffer, &video_str->au.video, &video_str->start,
		&bytes_output, mux_rate, audio_buffer_size, video_buffer_size,
		packet_data_size, marker_pack, video_str->nstream, streams);

	    /* video fehlermeldung */
	    /* video error message */
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    status_message (STATUS_VIDEO_TIME_OUT);
#if 0
	    /* status info */
	    status_info (nsec_a, ++nsec_v, nsec_p, bytes_output, 
			 buffer_space(&video_buffer),
			 buffer_space(&audio_buffer),verbose);
#endif
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    continue;
	}

	/* FALL: Audio Buffer nicht OK				*/
	/*       Video Buffer nicht OK				*/
	/* CASE: Audio Buffer NOT OK				*/
	/*	 Video Buffer NOT OK				*/

	/* else */
	{
	    fputs("CASE:5\n", debug_file);
	    /* padding packet schicken */
	    /* write out padding packet */
	    output_padding (&current_SCR, ostream, &pack, &sys_header,
		&sector, &bytes_output, mux_rate, audio_buffer_size, 
		video_buffer_size,packet_data_size, marker_pack, /*FIXME*/
		streams);

	    /* status info */
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
#if 0
	    status_info (nsec_a, nsec_v, ++nsec_p, bytes_output, 
			 buffer_space(&video_buffer),
			 buffer_space(&audio_buffer),verbose);
#endif
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	}
    }


    /* ISO 11172 END CODE schreiben				*/
    /* write out ISO 11172 END CODE				*/
    index = sector.buf;

    *(index++) = (unsigned char)((ISO11172_END)>>24);
    *(index++) = (unsigned char)((ISO11172_END & 0x00ff0000)>>16);
    *(index++) = (unsigned char)((ISO11172_END & 0x0000ff00)>>8);
    *(index++) = (unsigned char)(ISO11172_END & 0x000000ff);

    fwrite (sector.buf, sizeof (unsigned char), 4, ostream);
    bytes_output += 4;

#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
    /* status info*/
#if 0
    status_info (nsec_a, nsec_v, nsec_p, bytes_output, 
		 buffer_space(&video_buffer),
		 buffer_space(&audio_buffer),verbose);
#endif
    if (!verbose) printf ("\n");
    status_footer ();
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
    /* Schliesse alle Ein- und Ausgabefiles			*/
    /* close all In- and Outputfiles				*/

    /* loeschen der temporaeren Files */
    /* delete tmp files	*/

    fclose (ostream);

    for (stream = streams; stream; stream=stream->next){
	    fclose(stream->file);
	    fclose(stream->units);
	    unlink (stream->units_name);
    }


    printf ("\nDone.  Be sure to visit heroinewarrior.com for updates.\n\n");

#ifdef TIMER
    gettimeofday (&tp_global_end, NULL);
    global_sec = 10*(tp_global_end.tv_sec - tp_global_start.tv_sec);
    global_usec= 10*(tp_global_end.tv_usec - tp_global_start.tv_usec);
    global_sec += (global_usec / 100000);
    total_sec *= 10;
    total_sec  += (total_usec  / 100000);

    printf ("Timing global: %10.1f secs\n",(float)global_sec/10.);
    printf ("Timing IO    : %10.1f secs\n",(float)total_sec/10.);
#endif
    
}


/******************************************************************
	Next_Video_Access_Unit
	holt aus dem TMP File, der die Info's ueber die Access
	Units enthaelt, die jetzt gueltige Info her. Nach
	dem Erstellen des letzten Packs sind naemlich eine
	bestimmte Anzahl Bytes und damit AU's eingelesen worden.

	gets information for the next access unit from the tmp
	file
******************************************************************/

void next_video_access_unit (buffer, video_au, bytes_left, vunits_info,
			picture_start, SCR_delay)
Buffer_struc *buffer;
Vaunit_struc *video_au;
unsigned int *bytes_left;
FILE *vunits_info;
unsigned char *picture_start;
Timecode_struc *SCR_delay;

{

	int i;

	if (*bytes_left == 0)
	    return;

	while (video_au->length < *bytes_left)
	{
	    queue_buffer (buffer, video_au->length, &video_au->DTS);
	    *bytes_left -= video_au->length;
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    i=fread (video_au, sizeof(Vaunit_struc), 1, vunits_info);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    if (i != 1)
	    {
		empty_vaunit_struc (video_au);
		status_message(STATUS_VIDEO_END);
		return;
	    }
	    *picture_start = TRUE;
	    add_to_timecode (SCR_delay, &video_au->DTS);
	    add_to_timecode (SCR_delay, &video_au->PTS);
	};

	if (video_au->length > *bytes_left)
	{
	    queue_buffer (buffer, *bytes_left, &video_au->DTS);
	    video_au->length -= *bytes_left;
	    *picture_start = FALSE;
	} else
	if (video_au->length == *bytes_left)
	{
	    queue_buffer (buffer, *bytes_left, &video_au->DTS);
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    i=fread (video_au, sizeof(Vaunit_struc), 1, vunits_info);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    if (i != 1)
	    {
		empty_vaunit_struc (video_au);
		status_message(STATUS_VIDEO_END);
		return;
	    }
	    *picture_start = TRUE;
	    add_to_timecode (SCR_delay, &video_au->DTS);
	    add_to_timecode (SCR_delay, &video_au->PTS);
	};

}


/******************************************************************
	Output_Video
	generiert Pack/Sys_Header/Packet Informationen aus dem
	Video Stream und speichert den so erhaltenen Sektor ab.

	generates Pack/Sys_Header/Packet information from the
	video stream and writes out the new sector
******************************************************************/

void output_video (SCR, SCR_delay, vunits_info, istream_v, ostream,
		   pack, sys_header, sector, buffer, video_au,
		   picture_start, bytes_output, mux_rate,
		   audio_buffer_size, video_buffer_size,
		   packet_data_size, marker_pack, nstream, streams)

Timecode_struc *SCR;
Timecode_struc *SCR_delay;
FILE *vunits_info;
FILE *istream_v;
FILE *ostream;
Pack_struc *pack;
Sys_header_struc *sys_header;
Sector_struc *sector;
Buffer_struc *buffer;
Vaunit_struc *video_au;
unsigned char *picture_start;
unsigned int  *bytes_output;
unsigned int mux_rate;
unsigned long audio_buffer_size;
unsigned long video_buffer_size;
unsigned long packet_data_size;
unsigned char marker_pack;
unsigned char nstream;
stream_element *streams;

{

    unsigned int bytes_left;
    unsigned int temp;
    Pack_struc *pack_ptr;
    Sys_header_struc *sys_header_ptr;
    unsigned char timestamps;


    if (marker_pack)
    {
    	/* Wir generieren den Pack Header				*/
	/* let's generate pack header					*/
    	create_pack (pack, SCR, mux_rate);

    	/* Wir generieren den System Header				*/
	/* let's generate system header					*/
    	create_sys_header (sys_header, mux_rate, 1, 1, 1, 1, 1, 1,
			AUDIO_STR_0, 0, audio_buffer_size/128,
			VIDEO_STR_0, 1, video_buffer_size/1024, streams );
	pack_ptr = pack;
	sys_header_ptr = sys_header;
    } else
    {
	pack_ptr = NULL;
	sys_header_ptr = NULL;
    }

    /* Wir generieren das Packet				*/
    /* let's generate packet					*/

    /* faengt im Packet ein Bild an?				*/
    /* does a frame start in this packet?			*/
    
    /* FALL: Packet beginnt mit neuer Access Unit		*/
    /* CASE: Packet starts with new access unit			*/
    if (*picture_start)
    {
	if (video_au->type == BFRAME)
	    timestamps=TIMESTAMPS_PTS;
	else
	    timestamps=TIMESTAMPS_PTS_DTS;

	create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_v,VIDEO_STR_0+nstream,1, video_buffer_size/1024,
			TRUE, &video_au->PTS, &video_au->DTS,
		        timestamps);

	bytes_left = sector->length_of_packet_data;

	next_video_access_unit (buffer, video_au, &bytes_left, vunits_info,
				picture_start, SCR_delay);

    }

    /* FALL: Packet beginnt mit alter Access Unit, es kommt	*/
    /*       keine neue im selben Packet vor			*/
    /* CASE: Packet begins with old access unit, no new one	*/
    /*	     begins in the very same packet			*/
    else if (!(*picture_start) && (video_au->length >= packet_data_size))
    {
	create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_v,VIDEO_STR_0+nstream,1, video_buffer_size/1024,
			TRUE, NULL, NULL,
			TIMESTAMPS_NO );

	bytes_left = sector->length_of_packet_data;

	next_video_access_unit (buffer, video_au, &bytes_left, vunits_info,
				picture_start, SCR_delay);

    }

    /* FALL: Packet beginnt mit alter Access Unit, es kommt	*/
    /*       eine neue im selben Packet vor			*/
    /* CASE: Packet begins with old access unit, a new one	*/
    /*	     begins in the very same packet			*/
    else if (!(*picture_start) && (video_au->length < packet_data_size))
    {
	temp = video_au->length;
	queue_buffer (buffer, video_au->length, &video_au->DTS);

	/* gibt es ueberhaupt noch eine Access Unit ? */
	/* is there a new access unit anyway? */

#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	if (fread (video_au, sizeof(Vaunit_struc), 1, vunits_info)==1)
	{
	    if (video_au->type == BFRAME)
		timestamps=TIMESTAMPS_PTS;
	    else
		timestamps=TIMESTAMPS_PTS_DTS;

	    *picture_start = TRUE;
	    add_to_timecode (SCR_delay, &video_au->DTS);
	    add_to_timecode (SCR_delay, &video_au->PTS);
	    create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_v,VIDEO_STR_0+nstream,1, video_buffer_size/1024,
			TRUE, &video_au->PTS, &video_au->DTS,
			timestamps );
	bytes_left = sector->length_of_packet_data - temp;

	next_video_access_unit (buffer, video_au, &bytes_left, vunits_info,
				picture_start, SCR_delay);
	} else
	{
	    status_message(STATUS_VIDEO_END);
	    empty_vaunit_struc (video_au);
	    create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_v,VIDEO_STR_0+nstream,1, video_buffer_size/1024,
			TRUE, NULL, NULL,
			TIMESTAMPS_NO );
	};
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif

    }


    /* Sector auf Platte schreiben				*/
    /* write out sector						*/
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
    fwrite (sector->buf, sector->length_of_sector, 1, ostream);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
             total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
    *bytes_output += sector->length_of_sector;
	
}


/******************************************************************
	Next_Audio_Access_Unit
	holt aus dem TMP File, der die Info's ueber die Access
	Units enthaelt, die jetzt gueltige Info her. Nach
	dem Erstellen des letzten Packs sind naemlich eine
	bestimmte Anzahl Bytes und damit AU's eingelesen worden.

	gets information on access unit from the tmp file
******************************************************************/

void next_audio_access_unit (buffer, audio_au, bytes_left, aunits_info,
			audio_frame_start, SCR_delay)
Buffer_struc *buffer;
Aaunit_struc *audio_au;
unsigned int *bytes_left;
FILE *aunits_info;
unsigned char *audio_frame_start;
Timecode_struc *SCR_delay;

{

	int i;

	if (*bytes_left == 0)
	    return;

	while (audio_au->length < *bytes_left)
	{
	    queue_buffer (buffer, audio_au->length, &audio_au->PTS);
	    *bytes_left -= audio_au->length;
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    i=fread (audio_au, sizeof(Aaunit_struc), 1, aunits_info);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    if (i != 1)
	    {
		empty_aaunit_struc (audio_au);
		status_message(STATUS_AUDIO_END);
		return;
	    }
	    *audio_frame_start = TRUE;
	    add_to_timecode (SCR_delay, &audio_au->PTS);
	};

	if (audio_au->length > *bytes_left)
	{
	    queue_buffer (buffer, *bytes_left, &audio_au->PTS);
	    audio_au->length -= *bytes_left;
	    *audio_frame_start = FALSE;
	} else
	if (audio_au->length == *bytes_left)
	{
	    queue_buffer (buffer, *bytes_left, &audio_au->PTS);
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	    i=fread (audio_au, sizeof(Aaunit_struc), 1, aunits_info);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
	    if (i != 1)
	    {
		empty_aaunit_struc (audio_au);
		status_message(STATUS_AUDIO_END);
		return;
	    }
	    *audio_frame_start = TRUE;
	    add_to_timecode (SCR_delay, &audio_au->PTS);
	};

}

/******************************************************************
	Output_Audio
	erstellt Pack/Sys_Header/Packet Informationen aus dem
	Audio Stream und speichert den so erhaltenen Sector ab.

	generates Pack/Sys Header/Packet information from the
	audio stream and saves them into the sector
******************************************************************/

void output_audio (SCR, SCR_delay, aunits_info, istream_a, ostream,
		   pack, sys_header, sector, buffer, audio_au,
		   audio_frame_start, bytes_output, mux_rate,
		   audio_buffer_size, video_buffer_size,
		   packet_data_size, marker_pack, nstream, streams)

Timecode_struc *SCR;
Timecode_struc *SCR_delay;
FILE *aunits_info;
FILE *istream_a;
FILE *ostream;
Pack_struc *pack;
Sys_header_struc *sys_header;
Sector_struc *sector;
Buffer_struc *buffer;
Aaunit_struc *audio_au;
unsigned char *audio_frame_start;
unsigned int  *bytes_output;
unsigned int mux_rate;
unsigned long audio_buffer_size;
unsigned long video_buffer_size;
unsigned long packet_data_size;
unsigned char marker_pack;
unsigned char nstream;
stream_element *streams;

{

    unsigned int bytes_left;
    unsigned int temp;
    Pack_struc *pack_ptr;
    Sys_header_struc *sys_header_ptr;

    if (marker_pack)
    {
    	/* Wir generieren den Pack Header				*/
	/* let's generate pack header					*/
    	create_pack (pack, SCR, mux_rate);

    	/* Wir generieren den System Header				*/
	/* let's generate system header					*/
    	create_sys_header (sys_header, mux_rate, 1, 1, 1, 1, 1, 1,
			AUDIO_STR_0, 0, audio_buffer_size/128,
			VIDEO_STR_0, 1, video_buffer_size/1024, streams );
	pack_ptr = pack;
	sys_header_ptr = sys_header;
    }
    else
    {
	pack_ptr = NULL;
	sys_header_ptr = NULL;
    }

    /* Wir generieren das Packet				*/
    /* Let's generate packet					*/

    /* faengt im Packet ein Audio Frame an?			*/
    /* does a audio frame start in this packet?			*/

    /* FALL: Packet beginnt mit neuer Access Unit			*/
    /* CASE: packet starts with new access unit			*/
    if (*audio_frame_start)
    {
	create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_a,AUDIO_STR_0+nstream, 0, audio_buffer_size/128,
			TRUE, &audio_au->PTS, NULL,
			TIMESTAMPS_PTS);

	bytes_left = sector->length_of_packet_data;

	next_audio_access_unit (buffer, audio_au, &bytes_left, aunits_info,
				audio_frame_start, SCR_delay);
    }

    /* FALL: Packet beginnt mit alter Access Unit, es kommt	*/
    /*       keine neue im selben Packet vor			*/
    /* CASE: packet starts with old access unit, no new one	*/
    /*       starts in this very same packet			*/
    else if (!(*audio_frame_start) && (audio_au->length >= packet_data_size))
    {
	create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_a,AUDIO_STR_0+nstream, 0, audio_buffer_size/128,
			TRUE, NULL, NULL,
			TIMESTAMPS_NO );

	bytes_left = sector->length_of_packet_data;

	next_audio_access_unit (buffer, audio_au, &bytes_left, aunits_info,
				audio_frame_start, SCR_delay);
    }

    /* FALL: Packet beginnt mit alter Access Unit, es kommt	*/
    /*       eine neue im selben Packet vor			*/
    /* CASE: packet starts with old access unit, a new one	*/
    /*       starts in this very same packet			*/
    else if (!(*audio_frame_start) && (audio_au->length < packet_data_size))
    {
	temp = audio_au->length;
	queue_buffer (buffer, audio_au->length, &audio_au->PTS);

	/* gibt es ueberhaupt noch eine Access Unit ? */
	/* is there another access unit anyway ? */

#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
	if (fread (audio_au, sizeof(Aaunit_struc), 1, aunits_info)==1)
	{
	    *audio_frame_start = TRUE;
	    add_to_timecode (SCR_delay, &audio_au->PTS);
	    create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_a,AUDIO_STR_0+nstream, 0, audio_buffer_size/128,
			TRUE, &audio_au->PTS, NULL,
			TIMESTAMPS_PTS );

	bytes_left = sector->length_of_packet_data - temp;

	next_audio_access_unit (buffer, audio_au, &bytes_left, aunits_info,
				audio_frame_start, SCR_delay);
	} else
	{
	    status_message(STATUS_AUDIO_END);
	    empty_aaunit_struc (audio_au);
	    create_sector (sector, pack_ptr, sys_header_ptr,
			packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
			istream_a,AUDIO_STR_0+nstream, 0, audio_buffer_size/128,
			TRUE, NULL, NULL,
			TIMESTAMPS_NO );
	};
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif


    }

    /* Sector auf Platte schreiben				*/
    /* write out sector onto disk				*/
#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
    fwrite (sector->buf, sector->length_of_sector, 1, ostream);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
    *bytes_output += sector->length_of_sector;

	
}

/******************************************************************
	Output_Padding
	erstellt Pack/Sys_Header/Packet Informationen zu einem
	Padding-Stream und speichert den so erhaltenen Sector ab.

	generates Pack/Sys Header/Packet information for a 
	padding stream and saves the sector
******************************************************************/

void output_padding (SCR,  ostream,
		   pack, sys_header, sector, bytes_output, mux_rate,
		   audio_buffer_size, video_buffer_size,
		   packet_data_size, marker_pack, streams)

Timecode_struc *SCR;
FILE *ostream;
Pack_struc *pack;
Sys_header_struc *sys_header;
Sector_struc *sector;
unsigned int  *bytes_output;
unsigned int mux_rate;
unsigned long audio_buffer_size;
unsigned long video_buffer_size;
unsigned long packet_data_size;
unsigned char marker_pack;
stream_element *streams;

{
    unsigned int bytes_left;
    unsigned int temp;
    Pack_struc *pack_ptr;
    Sys_header_struc *sys_header_ptr;

    if (marker_pack)
    {
    	/* Wir generieren den Pack Header				*/
	/* let's generate the pack header				*/
    	create_pack (pack, SCR, mux_rate);

    	/* Wir generieren den System Header				*/
	/* let's generate the system header				*/
    	create_sys_header (sys_header, mux_rate, 1, 1, 1, 1, 1, 1,
			AUDIO_STR_0, 0, audio_buffer_size/128,
			VIDEO_STR_0, 1, video_buffer_size/1024, streams );
	pack_ptr = pack;
	sys_header_ptr = sys_header;
    }
    else
    {
	pack_ptr = NULL;
	sys_header_ptr = NULL;
    }

    /* Wir generieren das Packet				*/
    /* let's generate the packet				*/
    create_sector (sector, pack_ptr, sys_header_ptr,
		packet_data_size+PACKET_HEADER_SIZE+AFTER_PACKET_LENGTH,
		NULL, PADDING_STR, 0, 0,
		FALSE, NULL, NULL,
		TIMESTAMPS_NO );

#ifdef TIMER
            gettimeofday (&tp_start,NULL);
#endif 
    fwrite (sector->buf, sector->length_of_sector*sizeof (unsigned char), 1,
	    ostream);
#ifdef TIMER
            gettimeofday (&tp_end,NULL);
            total_sec  += (tp_end.tv_sec - tp_start.tv_sec);
            total_usec += (tp_end.tv_usec - tp_start.tv_usec);
#endif
    *bytes_output += sector->length_of_sector;
	
}

