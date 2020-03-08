/* 
 *  ycrcb_to_rgb32.c
 *
 *     Copyright (C) Charles 'Buck' Krasic - April 2000
 *     Copyright (C) Erik Walthinsen - April 2000
 *
 *  This file is part of libdv, a free DV (IEC 61834/SMPTE 314M)
 *  decoder.
 *
 *  libdv is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your
 *  option) any later version.
 *   
 *  libdv is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 *  The libdv homepage is http://libdv.sourceforge.net/.  
 */

#include <glib.h>
#include "../colormodels.h"
#include "dv.h"
#include "libdv.h"

#define DVC_IMAGE_WIDTH 720
#define DVC_IMAGE_ROWOFFSET (DVC_IMAGE_WIDTH * pixel_size)

gint32 ylut[256];
gint32 impactcbr[256];
gint32 impactcbg[256];
gint32 impactcrg[256];
gint32 impactcrb[256];

static gint8 _dv_clamp[512];
static gint8 *dv_clamp;

static inline guint8 dv_clamp_c(gint d) { return d; }
static inline guint8 dv_clamp_y(gint d) {
  return dv_clamp[d + (128 - 16)];
} // dv_clamp_y


static cmodel_yuv_t yuv_table;

void dv_ycrcb_init()
{
	gint i;

	for(i = 0; i < 256; ++i) 
	{
    	ylut[i] = 298 * i;
    	impactcbr[i] = 409 * (gint8)(i);
    	impactcbg[i] = 100 * (gint8)(i);
    	impactcrg[i] = 208 * (gint8)(i);
    	impactcrb[i] = 516 * (gint8)(i);
	}
	dv_clamp = _dv_clamp+128;

	for(i = -128; i < (256 + 128); i++)
	{
    	if(i < 0) dv_clamp[i] = 0;
    	else if(i > 255) dv_clamp[i] = 255;
    	else dv_clamp[i] = i;
	} // for 
	
	cmodel_init_yuv(&yuv_table);
}


#define BLOCK_411_HEAD \
/* 8 rows */ \
	for(row = 0; row < 8; row++) \
	{ \
/* 4 dct blocks */ \
		for(i = 0; i < 4; i++) \
		{ \
    		Ytmp = Y[i]; \
/* two 4 pixel spans per dct block */ \
			for(j = 0; j < 2; j++) \
			{ \
				cb = CLAMP(*cb_frame, -128, 127) + 128; \
				cr = CLAMP(*cr_frame, -128, 127) + 128; \
				cb_frame++; \
				cr_frame++; \
/* 4x1 pixel */


#define BLOCK_411_TAIL \
			} \
    		Y[i] = Ytmp; \
		} \
/* 4 rows, 8 pixels */ \
    	output_rgb += DVC_IMAGE_ROWOFFSET - 4 * 8 * pixel_size;  \
		output_y += DVC_IMAGE_WIDTH - 4 * 8; \
		output_u += (DVC_IMAGE_WIDTH - 4 * 8) / 2; \
		output_v += (DVC_IMAGE_WIDTH - 4 * 8) / 2; \
	}

void dv_ycrcb_411_block(unsigned char **output_rows, 
	long mb_offset, 
	dv_block_t *bl, 
	int pixel_size, 
	int color_model)
{
	guint8 *base_rgb = output_rows[0] + mb_offset * pixel_size;
	guint8 *output_rgb = base_rgb;
	guint8 *output_y = output_rows[0] + mb_offset;
	guint8 *output_u = output_rows[1] + mb_offset / 2;
	guint8 *output_v = output_rows[2] + mb_offset / 2;
	int i, j, k, row;
	dv_coeff_t *Y[4], *cr_frame, *cb_frame;
	dv_coeff_t *Ytmp;
	int cb, cr, r, g, b, y, r_offset, g_offset, b_offset;

	Y[0] = bl[0].coeffs;
	Y[1] = bl[1].coeffs;
	Y[2] = bl[2].coeffs;
	Y[3] = bl[3].coeffs;
	cb_frame = bl[4].coeffs;
	cr_frame = bl[5].coeffs;

	switch(color_model)
	{
		case BC_YUV422P:
			BLOCK_411_HEAD
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_u++ = cr;
				*output_u++ = cr;
				*output_v++ = cb;
				*output_v++ = cb;
			BLOCK_411_TAIL
			break;

		case BC_YUV888:
			BLOCK_411_HEAD
			for(k = 0; k < 4; k++)
			{
				y = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_rgb++ = y;
				*output_rgb++ = cr;
				*output_rgb++ = cb;
			}
			BLOCK_411_TAIL
			break;

		case BC_RGB888:
			BLOCK_411_HEAD
			r_offset = yuv_table.vtor_tab[cb];
			g_offset = yuv_table.utog_tab[cr] + yuv_table.vtog_tab[cb];
			b_offset = yuv_table.utob_tab[cr];
			for(k = 0; k < 4; k++)
			{
				y = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				y <<= 16;
				r = (y + r_offset) >> 16;
				g = (y + g_offset) >> 16;
				b = (y + b_offset) >> 16;
				*output_rgb++ = CLAMP(r, 0, 255);
				*output_rgb++ = CLAMP(g, 0, 255);
				*output_rgb++ = CLAMP(b, 0, 255);
			}
			BLOCK_411_TAIL
			break;
	}
}

#define RIGHT_411_HEAD \
/* Two rows of blocks j, j+1 */ \
	for(j = 0; j < 4; j += 2) \
	{ \
    	cb_frame = bl[4].coeffs + (j * 2); \
    	cr_frame = bl[5].coeffs + (j * 2); \
 \
		for(row = 0; row < 8; row++) \
		{ \
			output_rgb = base_rgb; \
			output_y = base_y; \
			output_u = base_u; \
			output_v = base_v; \
 \
/* Two columns of blocks */ \
			for(i = 0; i < 2; i++) \
			{ \
				Ytmp = Y[j + i]; \
/* 4 spans of 2x2 pixels */ \
				for(col = 0; col < 8; col += 4) \
				{ \
					cb = CLAMP(*cb_frame, -128, 127) + 128; \
					cr = CLAMP(*cr_frame, -128, 127) + 128; \
					cb_frame++; \
					cr_frame++;


#define RIGHT_411_TAIL \
				} /* col */ \
 \
				Y[j + i] = Ytmp; \
			} /* i */ \
 \
			cb_frame += 4; \
			cr_frame += 4; \
			base_rgb += DVC_IMAGE_ROWOFFSET; \
			base_y += DVC_IMAGE_WIDTH; \
			base_u += DVC_IMAGE_WIDTH / 2; \
			base_v += DVC_IMAGE_WIDTH / 2; \
		} /* row */ \
	} /* j */



void dv_ycrcb_411_block_right(unsigned char **output_rows, 
	long mb_offset, 
	dv_block_t *bl, 
	int pixel_size, 
	int color_model)
{
	guint8 *base_rgb = output_rows[0] + mb_offset * pixel_size;
	guint8 *output_rgb;
	guint8 *base_y = output_rows[0] + mb_offset;
	guint8 *base_u = output_rows[1] + mb_offset / 2;
	guint8 *base_v = output_rows[2] + mb_offset / 2;
	guint8 *output_y, *output_u, *output_v;
	int i, j, k, row, col;
	dv_coeff_t *Y[4], *cr_frame, *cb_frame;
	dv_coeff_t *Ytmp;
	int cb, cr, r, g, b, y, r_offset, g_offset, b_offset;

	Y[0] = bl[0].coeffs;
	Y[1] = bl[1].coeffs;
	Y[2] = bl[2].coeffs;
	Y[3] = bl[3].coeffs;

	switch(color_model)
	{
		case BC_YUV422P:
			RIGHT_411_HEAD
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_y++ = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_u++ = cr;
				*output_u++ = cr;
				*output_v++ = cb;
				*output_v++ = cb;
			RIGHT_411_TAIL
			break;

		case BC_YUV888:
			RIGHT_411_HEAD
			for(k = 0; k < 4; k++)
			{
				y = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				*output_rgb++ = y;
				*output_rgb++ = cr;
				*output_rgb++ = cb;
			}
			RIGHT_411_TAIL
			break;

		case BC_RGB888:
			RIGHT_411_HEAD
// 4x1 pixels
			for(k = 0; k < 4; k++)
			{
				r_offset = yuv_table.vtor_tab[cb];
				g_offset = yuv_table.utog_tab[cr] + yuv_table.vtog_tab[cb];
				b_offset = yuv_table.utob_tab[cr];
				y = CLAMP(*Ytmp, -128, 127) + 128;
				Ytmp++;
				y <<= 16;
				r = (y + r_offset) >> 16;
				g = (y + g_offset) >> 16;
				b = (y + b_offset) >> 16;
				*output_rgb++ = CLAMP(r, 0, 255);
				*output_rgb++ = CLAMP(g, 0, 255);
				*output_rgb++ = CLAMP(b, 0, 255);
			} // k
			RIGHT_411_TAIL
			break;
	}
}

void dv_ycrcb_420_block(unsigned char **output_rows, 
	long mb_offset, 
	dv_block_t *bl, 
	int pixel_size,
	int color_model)
{
	guint8 *base_rgb = output_rows[0] + mb_offset * pixel_size;
	int i, j, k, row, col;
	guint8 *output_rgb0 = base_rgb;
	guint8 *output_rgb1 = base_rgb + DVC_IMAGE_ROWOFFSET;
	guint8 *output_y0 = output_rows[0] + mb_offset;
	guint8 *output_y1 = output_rows[0] + mb_offset + DVC_IMAGE_WIDTH;
	guint8 *output_u0 = output_rows[1] + mb_offset / 2;
	guint8 *output_u1 = output_rows[1] + (mb_offset + DVC_IMAGE_WIDTH) / 2;
	guint8 *output_v0 = output_rows[2] + mb_offset / 2;
	guint8 *output_v1 = output_rows[2] + (mb_offset + DVC_IMAGE_WIDTH) / 2;
	dv_coeff_t *Y[4];
	dv_coeff_t *cr_frame;
	dv_coeff_t *cb_frame;
	int cb, cr, r, g, b, y;

	Y[0] = bl[0].coeffs;
	Y[1] = bl[1].coeffs;
	Y[2] = bl[2].coeffs;
	Y[3] = bl[3].coeffs;
	cb_frame = bl[4].coeffs;
	cr_frame = bl[5].coeffs;

// Two rows of blocks j, j+1
	for (j = 0; j < 4; j += 2) 
	{ 
// 4 pairs of two rows
    	for (row = 0; row < 8; row+=2) 
		{ 
// Two columns of blocks
    		for (i = 0; i < 2; ++i) 
			{ 
        		int yindex = j + i;
        		dv_coeff_t *Ytmp0 = Y[yindex];
        		dv_coeff_t *Ytmp1 = Y[yindex] + 8;

// 4 spans of 2x2 pixels
				switch(color_model)
				{
					case BC_YUV422P:
        				for (col = 0; col < 4; ++col) 
						{
							cb = CLAMP(*cb_frame, -128, 127) + 128;
							cr = CLAMP(*cr_frame, -128, 127) + 128;
							cb_frame++;
							cr_frame++;
// 2x2 pixel
							*output_y0++ = CLAMP(*Ytmp0, -128, 127) + 128;
							Ytmp0++;
							*output_y0++ = CLAMP(*Ytmp0, -128, 127) + 128;
							Ytmp0++;
							*output_y1++ = CLAMP(*Ytmp1, -128, 127) + 128;
							Ytmp1++;
							*output_y1++ = CLAMP(*Ytmp1, -128, 127) + 128;
							Ytmp1++;
							*output_u0++ = cr;
							*output_u1++ = cr;
							*output_v0++ = cb;
							*output_v1++ = cb;
						}
						break;

					case BC_YUV888:
        				for (col = 0; col < 4; ++col) 
						{  
							cb = CLAMP(*cb_frame, -128, 127) + 128;
							cr = CLAMP(*cr_frame, -128, 127) + 128;
							cb_frame++;
							cr_frame++;
// 2x2 pixel
        					for (k = 0; k < 2; ++k) 
							{
								y = CLAMP(*Ytmp0, -128, 127) + 128;
								Ytmp0++;
								*(output_rgb0)++ = y;
								*(output_rgb0)++ = cr;
								*(output_rgb0)++ = cb;

								y = CLAMP(*Ytmp1, -128, 127) + 128;
								Ytmp1++;
								*(output_rgb1)++ = y;
								*(output_rgb1)++ = cr;
								*(output_rgb1)++ = cb;
							}
						}
						break;

					case BC_RGB888:
        				for (col = 0; col < 4; ++col) 
						{  
							cb = CLAMP(*cb_frame, -128, 127);
							cr = CLAMP(*cr_frame, -128, 127);
							cb_frame++;
							cr_frame++;
// 2x2 pixel
        					for (k = 0; k < 2; ++k) 
							{
								y = CLAMP(*Ytmp0, -128, 127) + 128;
								Ytmp0++;
								y <<= 16;
								r = (y + yuv_table.vtor[cb]) >> 16;
								g = (y + yuv_table.utog[cr] + yuv_table.vtog[cb]) >> 16;
								b = (y + yuv_table.utob[cr]) >> 16;
								*output_rgb0++ = CLAMP(r, 0, 255);
								*output_rgb0++ = CLAMP(g, 0, 255);
								*output_rgb0++ = CLAMP(b, 0, 255);

								y = CLAMP(*Ytmp1, -128, 127) + 128;
								Ytmp1++;
								y <<= 16;
								r = (y + yuv_table.vtor[cb]) >> 16;
								g = (y + yuv_table.utog[cr] + yuv_table.vtog[cb]) >> 16;
								b = (y + yuv_table.utob[cr]) >> 16;
								*output_rgb1++ = CLAMP(r, 0, 255);
								*output_rgb1++ = CLAMP(g, 0, 255);
								*output_rgb1++ = CLAMP(b, 0, 255);
							}
						}
						break;
				}
				Y[yindex] = Ytmp1;
			}
    		output_rgb0 += 2 * DVC_IMAGE_ROWOFFSET - 2 * 8 * pixel_size;
    		output_rgb1 += 2 * DVC_IMAGE_ROWOFFSET - 2 * 8 * pixel_size;
			output_y0 += 2 * DVC_IMAGE_WIDTH - 2 * 8;
			output_y1 += 2 * DVC_IMAGE_WIDTH - 2 * 8;
			output_u0 += (2 * DVC_IMAGE_WIDTH - 2 * 8) / 2;
			output_u1 += (2 * DVC_IMAGE_WIDTH - 2 * 8) / 2;
			output_v0 += (2 * DVC_IMAGE_WIDTH - 2 * 8) / 2;
			output_v1 += (2 * DVC_IMAGE_WIDTH - 2 * 8) / 2;
		}
	}
}
