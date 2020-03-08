/* mpeg2enc.c, main() and parameter file reading                            */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define GLOBAL_ /* used by global.h */
#include "config.h"
#include "global.h"

/* private prototypes */
static void init _ANSI_ARGS_((void));
static void readcmdline _ANSI_ARGS_((int argc, char *argv[]));
static void readquantmat _ANSI_ARGS_((void));


int HorzMotionCode(int i)
{
  if (i < 8)
    return 1;
  if (i < 16)
    return 2;
  if (i < 32)
    return 3;
  if ((i < 64) || (constrparms))
    return 4;
  if (i < 128)
    return 5;
  if (i < 256)
    return 6;
  if ((i < 512) || (level == 10))
    return 7;
  if ((i < 1024) || (level == 8))
    return 8;
  if (i < 2048)
    return 9;
  return 1;
}

int VertMotionCode(int i)
{
  if (i < 8)
    return 1;
  if (i < 16)
    return 2;
  if (i < 32)
    return 3;
  if ((i < 64) || (level == 10) || (constrparms))
    return 4;
  return 5;
}

static void init()
{
  int i, size;
  static int block_count_tab[3] = {6,8,12};

  initbits();
  init_fdct();
  init_idct();

  /* round picture dimensions to nearest multiple of 16 or 32 */
  mb_width = (horizontal_size+15)/16;
  mb_height = prog_seq ? (vertical_size+15)/16 : 2*((vertical_size+31)/32);
  mb_height2 = fieldpic ? mb_height>>1 : mb_height; /* for field pictures */
  width = 16*mb_width;
  height = 16*mb_height;

  chrom_width = (chroma_format==CHROMA444) ? width : width>>1;
  chrom_height = (chroma_format!=CHROMA420) ? height : height>>1;

  height2 = fieldpic ? height>>1 : height;
  width2 = fieldpic ? width<<1 : width;
  chrom_width2 = fieldpic ? chrom_width<<1 : chrom_width;
  
  block_count = block_count_tab[chroma_format-1];

  /* clip table */
  if (!(clp = (unsigned char *)malloc(1024)))
    error("malloc failed\n");
  clp+= 384;
  for (i=-384; i<640; i++)
    clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  for (i=0; i<3; i++)
  {
    size = (i==0) ? width*height : chrom_width*chrom_height;

    if (!(newrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(oldrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(auxframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(neworgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(oldorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(auxorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(predframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
  }

  mbinfo = (struct mbinfo *)malloc(mb_width*mb_height2*sizeof(struct mbinfo));

  if (!mbinfo)
    error("malloc failed\n");

  blocks =
    (short (*)[64])malloc(mb_width*mb_height2*block_count*sizeof(short [64]));

  if (!blocks)
    error("malloc failed\n");

  /* open statistics output file */
  if (statname[0]=='-')
    statfile = stdout;
  else if (!(statfile = fopen(statname,"w")))
  {
    sprintf(errortext,"Couldn't create statistics output file %s",statname);
    error(errortext);
  }
}

void error(text)
char *text;
{
  fprintf(stderr,text);
  putc('\n',stderr);
  exit(1);
}

#define STRINGLEN 1024

static void init_params(char *path,
		int shm_id, 
		int inputsemid, 
		int outputsemid, 
		int width, 
		int height, 
		float framerate,
		int bitrate,
		int interlaced,
		int video_layer,
		int pixelsize)
{
	int i, j;
	int h, m, s, f;
	FILE *fd;
	char line[256];
// Master frame rate table must match decoder
	static double ratetab[]=
    	{24000.0/1001.0,  // Official rates
		24.0,
		25.0,
		30000.0/1001.0,
		30.0,
		50.0,
		60000.0/1001.0,
		60.0,

		1,           // Unofficial economy rates
		5,
		10,
		12, 
		15,
		0,
		0};

// VBV buffer size limits
	int vbvlim[4] = { 597, 448, 112, 29 };
	long total_frame_rates = (sizeof(ratetab) / sizeof(double));

	quiet = 1;
	prog_seq = 0;  /* progressive_sequence is faster */
	mpeg1 = 0;  /* ISO/IEC 11172-2 stream */
	strcpy(out_path, path);
	prog_seq = !interlaced;
	mpeg1 = video_layer == 1;
	bit_rate = bitrate * 1000;
	pixel_size = pixelsize;
	input_sem_id = inputsemid;
	output_sem_id = outputsemid;
	first_frame_number = 0;
	frames_buffered = 0;
	input_shm_id = shm_id;
	end_of_input = 0;
// Arbitrary large number
	nframes = 100;

/************************************************************************
 *                            BEGIN PARAMETER FILE
 ************************************************************************/

/* To eliminate the user hassle we replaced the parameter file with hard coded constants. */
	strcpy(tplref,             "-");  /* name of intra quant matrix file	 ("-": default matrix) */
	strcpy(iqname,             "-");  /* name of intra quant matrix file	 ("-": default matrix) */
	strcpy(niqname,            "-");  /* name of non intra quant matrix file ("-": default matrix) */
	strcpy(statname,           "/dev/null");  /* name of statistics file ("-": stdout ) */
	inputtype =                3;  /* input picture file format: 0=*.Y,*.U,*.V, 1=*.yuv, 2=*.ppm */
	frame0 =                   0;  /* number of first frame */
	h = m = s = f =            0;  /* timecode of first frame */
	N =                        15;  /* N (# of frames in GOP) */
// Changing B crashes it.
	M =                        3;  /* M (I/P frame distance) */
	fieldpic =                 0;  /* 0:frame pictures, 1:field pictures */
	horizontal_size =          width;
	vertical_size =            height;
	aspectratio =              1;  /* aspect_ratio_information 1=square pel, 2=4:3, 3=16:9, 4=2.11:1 */
	low_delay =                0;  /* low_delay  */
	constrparms =              0;  /* constrained_parameters_flag */
	profile =                  4;  /* Profile ID: Simple = 5, Main = 4, SNR = 3, Spatial = 2, High = 1 */
	level =                    4;  /* Level ID:   Low = 10, Main = 8, High 1440 = 6, High = 4		   */
	chroma_format =            1;  /* chroma_format: 1=4:2:0, 2=4:2:2, 3=4:4:4   LibMPEG2 only does 1 */
	video_format =             2;  /* video_format: 0=comp., 1=PAL, 2=NTSC, 3=SECAM, 4=MAC, 5=unspec. */
	color_primaries =          5;  /* color_primaries */
	transfer_characteristics = 5;  /* transfer_characteristics */
	matrix_coefficients =      4;  /* matrix_coefficients (not used) */
	display_horizontal_size =  horizontal_size;
	display_vertical_size =    vertical_size;
	dc_prec =                  0;  /* intra_dc_precision (0: 8 bit, 1: 9 bit, 2: 10 bit, 3: 11 bit */
	topfirst =                 1;  /* top_field_first */
	frame_pred_dct_tab[0] =    0;  /* frame_pred_frame_dct (I P B) */
	frame_pred_dct_tab[1] =    0;  /* frame_pred_frame_dct (I P B) */
	frame_pred_dct_tab[2] =    0;  /* frame_pred_frame_dct (I P B) */
	conceal_tab[0] =           0;  /* concealment_motion_vectors (I P B) */
	conceal_tab[1] =           0;  /* concealment_motion_vectors (I P B) */
	conceal_tab[2] =           0;  /* concealment_motion_vectors (I P B) */
	qscale_tab[0] =            1;  /* q_scale_type  (I P B) */
	qscale_tab[1] =            1;  /* q_scale_type  (I P B) */
	qscale_tab[2] =            1;  /* q_scale_type  (I P B) */
	intravlc_tab[0] =          1;  /* intra_vlc_format (I P B)*/
	intravlc_tab[1] =          0;  /* intra_vlc_format (I P B)*/
	intravlc_tab[2] =          0;  /* intra_vlc_format (I P B)*/
	altscan_tab[0] =           0;  /* alternate_scan (I P B) */
	altscan_tab[1] =           0;  /* alternate_scan (I P B) */
	altscan_tab[2] =           0;  /* alternate_scan (I P B) */
	repeatfirst =              0;  /* repeat_first_field */
	prog_frame =               0;  /* progressive_frame */
/* P:  forw_hor_f_code forw_vert_f_code search_width/height */
   motion_data = (struct motion_data *)malloc(3 * sizeof(struct motion_data));
	motion_data[0].forw_hor_f_code =  2;
	motion_data[0].forw_vert_f_code = 2;
	motion_data[0].sxf =              11;
	motion_data[0].syf =              11;
/* B1: forw_hor_f_code forw_vert_f_code search_width/height */
	motion_data[1].forw_hor_f_code =  1;
	motion_data[1].forw_vert_f_code = 1;
	motion_data[1].sxf =              3;
	motion_data[1].syf =              3;
/* B1: back_hor_f_code back_vert_f_code search_width/height */
	motion_data[1].back_hor_f_code =  1;
	motion_data[1].back_vert_f_code = 1;
	motion_data[1].sxb =              7;
	motion_data[1].syb =              7;
/* B2: forw_hor_f_code forw_vert_f_code search_width/height */
	motion_data[2].forw_hor_f_code =  1;
	motion_data[2].forw_vert_f_code = 1;
	motion_data[2].sxf =              7;
	motion_data[2].syf =              7;
/* B2: back_hor_f_code back_vert_f_code search_width/height */
	motion_data[2].back_hor_f_code =  1;
	motion_data[2].back_vert_f_code = 1;
	motion_data[2].sxb =              3;
	motion_data[2].syb =              3;

/************************************************************************
 *                                END PARAMETER FILE
 ************************************************************************/

    if (mpeg1) prog_seq = 1;

/* Auto settings (from bbMPEG http://members.home.net/beyeler/bbmpeg.html) */
    for(i = 1; i < M; i++)
    {
    	 motion_data[i].sxf = 3 * i;
    	 motion_data[i].forw_hor_f_code = HorzMotionCode(motion_data[i].sxf);
    	 motion_data[i].syf = 3 * i;
    	 motion_data[i].forw_vert_f_code = VertMotionCode(motion_data[i].syf);
    	 motion_data[i].sxb = 3 * (M - 1);
    	 motion_data[i].back_hor_f_code = HorzMotionCode(motion_data[i].sxb);
    	 motion_data[i].syb = 3 * (M - 1);
    	 motion_data[i].back_vert_f_code = VertMotionCode(motion_data[i].syb);
    }

	motion_data[0].sxf = 3 * M;
	motion_data[0].forw_hor_f_code = HorzMotionCode(motion_data[0].sxf);
	motion_data[0].syf = 3 * M;
	motion_data[0].forw_vert_f_code = VertMotionCode(motion_data[0].syf);

    vbv_buffer_size = floor(((double)bit_rate * 0.20343) / 16384.0);

    if(vbv_buffer_size > vbvlim[(level - 4) >> 1])
        vbv_buffer_size = vbvlim[(level - 4) >> 1];

/* Set up Quicktime buffers */
	frame_rate = framerate;

// Get frame rate code from literal frame rate
	for(i = 0; i < total_frame_rates; i++)
	{
		if(fabs(frame_rate - ratetab[i]) < 0.001) frame_rate_code = i + 1;
	}

	for(i = 0; i < READAHEAD; i++)
	{
		frame_buffer[i] = calloc(1, horizontal_size * vertical_size * pixel_size);
		row_pointers[i] = malloc(sizeof(unsigned char*) * vertical_size);
		for(j = 0; j < vertical_size; j++) row_pointers[i][j] = &frame_buffer[i][horizontal_size * pixel_size * j];
	}

/* make flags boolean (x!=0 -> x=1) */
  mpeg1 = !!mpeg1;
  fieldpic = !!fieldpic;
  low_delay = !!low_delay;
  constrparms = !!constrparms;
  prog_seq = !!prog_seq;
  topfirst = !!topfirst;

  for (i = 0; i < 3; i++)
  {
    frame_pred_dct_tab[i] = !!frame_pred_dct_tab[i];
    conceal_tab[i] = !!conceal_tab[i];
    qscale_tab[i] = !!qscale_tab[i];
    intravlc_tab[i] = !!intravlc_tab[i];
    altscan_tab[i] = !!altscan_tab[i];
  }
  repeatfirst = !!repeatfirst;
  prog_frame = !!prog_frame;

  /* make sure MPEG specific parameters are valid */
  range_checks();

  /* timecode -> frame number */
  tc0 = h;
  tc0 = 60*tc0 + m;
  tc0 = 60*tc0 + s;
  tc0 = (int)(frame_rate+0.5)*tc0 + f;

  if (!mpeg1)
  {
    profile_and_level_checks();
  }
  else
  {
    /* MPEG-1 */
    if (constrparms)
    {
      if (horizontal_size>768
          || vertical_size>576
          || ((horizontal_size+15)/16)*((vertical_size+15) / 16) > 396
          || ((horizontal_size+15)/16)*((vertical_size+15) / 16)*frame_rate>396*25.0
          || frame_rate>30.0)
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
        constrparms = 0;
      }
    }

    if (constrparms)
    {
      for (i=0; i<M; i++)
      {
        if (motion_data[i].forw_hor_f_code>4)
        {
          if (!quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          constrparms = 0;
          break;
        }

        if (motion_data[i].forw_vert_f_code>4)
        {
          if (!quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          constrparms = 0;
          break;
        }

        if (i!=0)
        {
          if (motion_data[i].back_hor_f_code>4)
          {
            if (!quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            constrparms = 0;
            break;
          }

          if (motion_data[i].back_vert_f_code>4)
          {
            if (!quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            constrparms = 0;
            break;
          }
        }
      }
    }
  }

  /* relational checks */

  if (mpeg1)
  {
    if (!prog_seq)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting progressive_sequence = 1\n");
      prog_seq = 1;
    }

    if (chroma_format!=CHROMA420)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting chroma_format = 1 (4:2:0)\n");
      chroma_format = CHROMA420;
    }

    if (dc_prec!=0)
    {
      if (!quiet)
        fprintf(stderr,"Warning: setting intra_dc_precision = 0\n");
      dc_prec = 0;
    }

    for (i=0; i<3; i++)
      if (qscale_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting qscale_tab[%d] = 0\n",i);
        qscale_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (intravlc_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting intravlc_tab[%d] = 0\n",i);
        intravlc_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (altscan_tab[i])
      {
        if (!quiet)
          fprintf(stderr,"Warning: setting altscan_tab[%d] = 0\n",i);
        altscan_tab[i] = 0;
      }
  }

  if (!mpeg1 && constrparms)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
    constrparms = 0;
  }

  if (prog_seq && !prog_frame)
  {
     if (!quiet)
       fprintf(stderr,"Warning: setting progressive_frame = 1\n");
    prog_frame = 1;
  }

  if (prog_frame && fieldpic)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting field_pictures = 0\n");
    fieldpic = 0;
  }

  if (!prog_frame && repeatfirst)
  {
    if (!quiet)
      fprintf(stderr,"Warning: setting repeat_first_field = 0\n");
    repeatfirst = 0;
  }

  if (prog_frame)
  {
    for (i=0; i<3; i++)
      if (!frame_pred_dct_tab[i])
      {
         if (!quiet)
           fprintf(stderr,"Warning: setting frame_pred_frame_dct[%d] = 1\n",i);
        frame_pred_dct_tab[i] = 1;
      }
  }

  if (prog_seq && !repeatfirst && topfirst)
  {
     if (!quiet)
       fprintf(stderr,"Warning: setting top_field_first = 0\n");
    topfirst = 0;
  }

  /* search windows */
  for (i=0; i<M; i++)
  {
    if (motion_data[i].sxf > (4<<motion_data[i].forw_hor_f_code)-1)
    {
      if (!quiet)
        fprintf(stderr,
          "Warning: reducing forward horizontal search width to %d\n",
          (4<<motion_data[i].forw_hor_f_code)-1);
      motion_data[i].sxf = (4<<motion_data[i].forw_hor_f_code)-1;
    }

    if (motion_data[i].syf > (4<<motion_data[i].forw_vert_f_code)-1)
    {
      if (!quiet)
        fprintf(stderr,
          "Warning: reducing forward vertical search width to %d\n",
          (4<<motion_data[i].forw_vert_f_code)-1);
      motion_data[i].syf = (4<<motion_data[i].forw_vert_f_code)-1;
    }

    if (i!=0)
    {
      if (motion_data[i].sxb > (4<<motion_data[i].back_hor_f_code)-1)
      {
        if (!quiet)
          fprintf(stderr,
            "Warning: reducing backward horizontal search width to %d\n",
            (4<<motion_data[i].back_hor_f_code)-1);
        motion_data[i].sxb = (4<<motion_data[i].back_hor_f_code)-1;
      }

      if (motion_data[i].syb > (4<<motion_data[i].back_vert_f_code)-1)
      {
        if (!quiet)
          fprintf(stderr,
            "Warning: reducing backward vertical search width to %d\n",
            (4<<motion_data[i].back_vert_f_code)-1);
        motion_data[i].syb = (4<<motion_data[i].back_vert_f_code)-1;
      }
    }
  }

}

static void readquantmat()
{
  int i,v;
  FILE *fd;

  if (iqname[0]=='-')
  {
    /* use default intra matrix */
    load_iquant = 0;
    for (i=0; i<64; i++)
      intra_q[i] = default_intra_quantizer_matrix[i];
  }
  else
  {
    /* read customized intra matrix */
    load_iquant = 1;
    if (!(fd = fopen(iqname,"r")))
    {
      sprintf(errortext,"Couldn't open quant matrix file %s",iqname);
      error(errortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      intra_q[i] = v;
    }

    fclose(fd);
  }

  if (niqname[0]=='-')
  {
    /* use default non-intra matrix */
    load_niquant = 0;
    for (i=0; i<64; i++)
      inter_q[i] = 16;
  }
  else
  {
    /* read customized non-intra matrix */
    load_niquant = 1;
    if (!(fd = fopen(niqname,"r")))
    {
      sprintf(errortext,"Couldn't open quant matrix file %s",niqname);
      error(errortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      inter_q[i] = v;
    }

    fclose(fd);
  }
}

int encode_mpeg_video(char *path,
		int shm_id, 
		int inputsemid, 
		int outputsemid, 
		int width, 
		int height, 
		float framerate,
		int bitrate,
		int interlaced,
		int video_layer,
		int pixelsize)
{
	int pid = fork();
	if(pid) return pid;
	
	init_params(path,
		shm_id, 
		inputsemid, 
		outputsemid, 
		width, 
		height, 
		framerate,
		bitrate,
		interlaced,
		video_layer,
		pixelsize);

/* read quantization matrices */
    readquantmat();

/* open output file */
	if (!(outfile=fopen(out_path, "wb")))
	{
    	sprintf(errortext,"Couldn't create output file %s", out_path);
    	error(errortext);
	}

	init();
	putseq();

	fclose(outfile);
	fclose(statfile);

	return 0;
}
