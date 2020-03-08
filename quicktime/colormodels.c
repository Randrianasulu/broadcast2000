#include "colormodels.h"
#include <glib.h>
#include <stdlib.h>


static cmodel_yuv_t *yuv_table = 0;

void cmodel_init_yuv(cmodel_yuv_t *yuv_table)
{
	int i;
	for(i = 0; i < 256; i++)
	{
/* compression */
		yuv_table->rtoy_tab[i] = (long)( 0.2990 * 65536 * i);
		yuv_table->rtou_tab[i] = (long)(-0.1687 * 65536 * i);
		yuv_table->rtov_tab[i] = (long)( 0.5000 * 65536 * i);

		yuv_table->gtoy_tab[i] = (long)( 0.5870 * 65536 * i);
		yuv_table->gtou_tab[i] = (long)(-0.3320 * 65536 * i);
		yuv_table->gtov_tab[i] = (long)(-0.4187 * 65536 * i);

		yuv_table->btoy_tab[i] = (long)( 0.1140 * 65536 * i);
		yuv_table->btou_tab[i] = (long)( 0.5000 * 65536 * i) + 0x800000;
		yuv_table->btov_tab[i] = (long)(-0.0813 * 65536 * i) + 0x800000;
	}

	yuv_table->vtor = &(yuv_table->vtor_tab[128]);
	yuv_table->vtog = &(yuv_table->vtog_tab[128]);
	yuv_table->utog = &(yuv_table->utog_tab[128]);
	yuv_table->utob = &(yuv_table->utob_tab[128]);
	for(i = -128; i < 128; i++)
	{
/* decompression */
		yuv_table->vtor[i] = (long)( 1.4020 * 65536 * i);
		yuv_table->vtog[i] = (long)(-0.7141 * 65536 * i);

		yuv_table->utog[i] = (long)(-0.3441 * 65536 * i);
		yuv_table->utob[i] = (long)( 1.7720 * 65536 * i);
	}
}


void cmodel_delete_yuv(cmodel_yuv_t *yuv_table)
{
}

int cmodel_is_planar(int colormodel)
{
	switch(colormodel)
	{
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
		case BC_YUV411P:      return 1; break;
	}
	return 0;
}

int cmodel_calculate_pixelsize(int colormodel)
{
	switch(colormodel)
	{
		case BC_TRANSPARENCY: return 1; break;
		case BC_COMPRESSED:   return 1; break;
		case BC_RGB8:         return 1; break;
		case BC_RGB565:       return 2; break;
		case BC_BGR565:       return 2; break;
		case BC_BGR888:       return 3; break;
		case BC_BGR8888:      return 4; break;
// Working bitmaps are packed to simplify processing
		case BC_RGB888:       return 3; break;
		case BC_RGBA8888:     return 4; break;
		case BC_RGB161616:    return 6; break;
		case BC_RGBA16161616: return 8; break;
		case BC_YUV888:       return 3; break;
		case BC_YUVA8888:     return 4; break;
		case BC_YUV161616:    return 6; break;
		case BC_YUVA16161616: return 8; break;
// Planar
		case BC_YUV420P:      return 1; break;
		case BC_YUV422P:      return 1; break;
		case BC_YUV422:       return 2; break;
		case BC_YUV411P:      return 1; break;
	}
	return 0;
}

#define RGB_TO_YUV(y, u, v, r, g, b) \
{ \
	y = ((yuv_table->rtoy_tab[r] + yuv_table->gtoy_tab[g] + yuv_table->btoy_tab[b]) >> 16); \
	u = ((yuv_table->rtou_tab[r] + yuv_table->gtou_tab[g] + yuv_table->btou_tab[b]) >> 16); \
	v = ((yuv_table->rtov_tab[r] + yuv_table->gtov_tab[g] + yuv_table->btov_tab[b]) >> 16); \
	RECLIP(y, 0, 255); \
	RECLIP(u, 0, 255); \
	RECLIP(v, 0, 255); \
}




// Pixel transfers
static inline void transfer_RGBA16161616_to_RGB8(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = ((guint16*)input)[3] >> 8;
	r = (unsigned int)((guint16*)input)[0] * a;
	g = (unsigned int)((guint16*)input)[1] * a;
	b = (unsigned int)((guint16*)input)[2] * a;

	*(*output) = (unsigned char)(((r & 0xc00000) >> 16) + 
				((g & 0xe00000) >> 18) + 
				((b & 0xe00000) >> 21));
	(*output)++;
}

static inline void transfer_RGBA16161616_to_RGB565(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = ((guint16*)input)[3] >> 8;
	r = (unsigned int)((guint16*)input)[0] * a;
	g = (unsigned int)((guint16*)input)[1] * a;
	b = (unsigned int)((guint16*)input)[2] * a;

	*(guint16*)(*output) = (guint16)(((r & 0xf80000) >> 8) + 
				((g & 0xfc0000) >> 13) + 
				((b & 0xf80000) >> 19));
	(*output) += 2;
}

static inline void transfer_RGBA16161616_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = ((guint16*)input)[3] >> 8;
	r = (unsigned int)((guint16*)input)[0] * a;
	g = (unsigned int)((guint16*)input)[1] * a;
	b = (unsigned int)((guint16*)input)[2] * a;

	(*output)[0] = (unsigned char)(b >> 16);
	(*output)[1] = (unsigned char)(g >> 16);
	(*output)[2] = (unsigned char)(r >> 16);
	(*output) += 3;
}

static inline void transfer_RGBA16161616_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = ((guint16*)input)[3] >> 8;
	r = ((guint16*)input)[0] * a;
	g = ((guint16*)input)[1] * a;
	b = ((guint16*)input)[2] * a;

	(*output)[0] = (unsigned char)(b >> 16);
	(*output)[1] = (unsigned char)(g >> 16);
	(*output)[2] = (unsigned char)(r >> 16);
	(*output) += 4;
}









static inline void transfer_RGBA8888_to_TRANSPARENCY(unsigned char *(*output), unsigned char *input, int (*bit_counter))
{
	if((*bit_counter) == 7) *(*output) = 0;

	if(input[3] < 127) 
	{
		*(*output) |= (unsigned char)1 << (7 - (*bit_counter));
	}

	if((*bit_counter) == 0)
	{
		(*output)++;
		(*bit_counter) = 7;
	}
	else
		(*bit_counter)--;
}

// These routines blend in a background color since they should be
// exclusively used for widgets.

static inline void transfer_RGBA8888_to_RGB8bg(unsigned char *(*output), unsigned char *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = (unsigned int)input[0] * a + bg_r * anti_a;
	g = (unsigned int)input[1] * a + bg_g * anti_a;
	b = (unsigned int)input[2] * a + bg_b * anti_a;
	*(*output) = (unsigned char)(((r & 0xc000) >> 8) + 
				((g & 0xe000) >> 10) + 
				((b & 0xe000) >> 13));
	(*output)++;
}

static inline void transfer_RGBA8888_to_RGB565bg(unsigned char *(*output), unsigned char *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = (unsigned int)input[0] * a + bg_r * anti_a;
	g = (unsigned int)input[1] * a + bg_g * anti_a;
	b = (unsigned int)input[2] * a + bg_b * anti_a;
	*(guint16*)(*output) = (guint16)((r & 0xf800) + 
				((g & 0xfc00) >> 5) + 
				((b & 0xf800) >> 11));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_BGR888bg(unsigned char *(*output), unsigned char *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;
	r = (unsigned int)input[0] * a + bg_r * anti_a;
	g = (unsigned int)input[1] * a + bg_g * anti_a;
	b = (unsigned int)input[2] * a + bg_b * anti_a;
	(*output)[0] = (b >> 8) + 1;
	(*output)[1] = (g >> 8) + 1;
	(*output)[2] = (r >> 8) + 1;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_BGR8888bg(unsigned char *(*output), unsigned char *input, int bg_r, int bg_g, int bg_b)
{
	unsigned int r, g, b, a, anti_a;
	a = input[3];
	anti_a = 255 - a;

	r = (unsigned int)input[0] * a + bg_r * anti_a;
	g = (unsigned int)input[1] * a + bg_g * anti_a;
	b = (unsigned int)input[2] * a + bg_b * anti_a;

	(*output)[0] = (b >> 8) + 1;
	(*output)[1] = (g >> 8) + 1;
	(*output)[2] = (r >> 8) + 1;
	(*output) += 4;
}







// These routines blend in a black background

static inline void transfer_RGBA8888_to_RGB8(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = (unsigned int)input[0] * a;
	g = (unsigned int)input[1] * a;
	b = (unsigned int)input[2] * a;
	*(*output) = (unsigned char)(((r & 0xc000) >> 8) + 
				((g & 0xe000) >> 10) + 
				((b & 0xe000) >> 13));
	(*output)++;
}

static inline void transfer_RGBA8888_to_RGB565(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = (unsigned int)input[0] * a;
	g = (unsigned int)input[1] * a;
	b = (unsigned int)input[2] * a;
	*(guint16*)(*output) = (guint16)((r & 0xf800) + 
				((g & 0xfc00) >> 5) + 
				((b & 0xf800) >> 11));
	(*output) += 2;
}

static inline void transfer_RGBA8888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = (unsigned int)input[0] * a;
	g = (unsigned int)input[1] * a;
	b = (unsigned int)input[2] * a;
	(*output)[0] = (b >> 8) + 1;
	(*output)[1] = (g >> 8) + 1;
	(*output)[2] = (r >> 8) + 1;
	(*output) += 3;
}

static inline void transfer_RGBA8888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	unsigned int r, g, b, a;
	a = input[3];
	r = (unsigned int)input[0] * a;
	g = (unsigned int)input[1] * a;
	b = (unsigned int)input[2] * a;
	(*output)[0] = (b >> 8) + 1;
	(*output)[1] = (g >> 8) + 1;
	(*output)[2] = (r >> 8) + 1;
	(*output) += 4;
}









static inline void transfer_RGB888_to_RGB8(unsigned char *(*output), unsigned char *input)
{
	*(*output) = (unsigned char)((input[0] & 0xc0) +
			    			 ((input[1] & 0xe0) >> 2) +
		 	    			 ((input[2] & 0xe0) >> 5));
	(*output)++;
}

static inline void transfer_RGB888_to_RGB565(unsigned char *(*output), unsigned char *input)
{
	guint16 r, g, b;
	r = *input++;
	g = *input++;
	b = *input;
	*(guint16*)(*output) = ((r & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((b & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_RGB888_to_BGR888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_RGB888_to_BGR8888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 4;
}

static inline void transfer_BGR8888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_BGR888_to_RGB888(unsigned char *(*output), unsigned char *input)
{
	(*output)[0] = input[2];
	(*output)[1] = input[1];
	(*output)[2] = input[0];
	(*output) += 3;
}

static inline void transfer_RGB888_to_YUV420P_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
	int y, u, v;

	RGB_TO_YUV(y, u, v, input[0], input[1], input[2]);

	output_y[output_column] = y;
	output_u[output_column / 2] = u;
	output_v[output_column / 2] = v;
}











#define YUV_TO_RGB(y, u, v, r, g, b) \
{ \
	r = ((y + yuv_table->vtor_tab[v]) >> 16); \
	g = ((y + yuv_table->utog_tab[u] + yuv_table->vtog_tab[v]) >> 16); \
	b = ((y + yuv_table->utob_tab[u]) >> 16); \
	RECLIP(r, 0, 255); \
	RECLIP(g, 0, 255); \
	RECLIP(b, 0, 255); \
}

static inline void transfer_YUV420P_to_RGB8(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int y, u, v;
	int r, g, b;
	
	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	*(*output) = (unsigned char)((r & 0xc0) +
			    			 ((g & 0xe0) >> 2) +
		 	    			 ((b & 0xe0) >> 5));
	(*output)++;
}

static inline void transfer_YUV420P_to_RGB565(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int y, u, v;
	int r, g, b;
	
	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	*(guint16*)(*output) = ((r & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((b & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV420P_to_BGR888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int y, u, v;
	int r, g, b;
	
	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 3;
}

static inline void transfer_YUV420P_to_RGB888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int y, u, v;
	int r, g, b;

	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output) += 3;
}

static inline void transfer_YUV420P_to_BGR8888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	int y, u, v;
	int r, g, b;

	y = (int)(*input_y) << 16;
	u = *input_u;
	v = *input_v;
	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 4;
}











static inline void transfer_YUV422P_to_YUV888(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v)
{
	(*output)[0] = *input_y;
	(*output)[1] = *input_u;
	(*output)[2] = *input_v;
	(*output) += 3;
}

static inline void transfer_YUV422P_to_YUV422(unsigned char *(*output), 
	unsigned char *input_y,
	unsigned char *input_u,
	unsigned char *input_v,
	int j)
{
// Store U and V for even pixels only
	if(!(j & 1))
	{
		(*output)[1] = *input_u;
		(*output)[3] = *input_v;
		(*output)[0] = *input_y;
	}
	else
// Store Y and advance output for odd pixels only
	{
		(*output)[2] = *input_y;
		(*output) += 4;
	}
}











static inline void transfer_YUV422_to_RGB8(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;

	u = input[1];
	v = input[3];
	YUV_TO_RGB(y, u, v, r, g, b)

	*(*output) = (unsigned char)((r & 0xc0) +
			    			 ((g & 0xe0) >> 2) +
		 	    			 ((b & 0xe0) >> 5));
	(*output)++;
}

static inline void transfer_YUV422_to_RGB565(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;
	u = input[1];
	v = input[3];
	YUV_TO_RGB(y, u, v, r, g, b)

	*(guint16*)(*output) = ((r & 0xf8) << 8)
			 + ((g & 0xfc) << 3)
			 + ((b & 0xf8) >> 3);
	(*output) += 2;
}

static inline void transfer_YUV422_to_BGR888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;
	u = input[1];
	v = input[3];
	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 3;
}

static inline void transfer_YUV422_to_RGB888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;
	u = input[1];
	v = input[3];
	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = r;
	(*output)[1] = g;
	(*output)[2] = b;
	(*output) += 3;
}

static inline void transfer_YUV422_to_YUV888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
// Even pixel
	if(!(column & 1))
		(*output)[0] = input[0];
	else
// Odd pixel
		(*output)[0] = input[2];

	(*output)[1] = input[1];
	(*output)[2] = input[3];
	(*output) += 3;
}

static inline void transfer_YUV422_to_BGR8888(unsigned char *(*output), 
	unsigned char *input, 
	int column)
{
	int y, u, v;
	int r, g, b;

// Even pixel
	if(!(column & 1))
		y = (int)(input[0]) << 16;
	else
// Odd pixel
		y = (int)(input[2]) << 16;
	u = input[1];
	v = input[3];

	YUV_TO_RGB(y, u, v, r, g, b)

	(*output)[0] = b;
	(*output)[1] = g;
	(*output)[2] = r;
	(*output) += 4;
}


static inline void transfer_YUV422_to_YUV422P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column)
{
// Store U and V for even pixels only
	if(!(output_column & 1))
	{
		output_y[output_column] = input[0];
		output_u[output_column / 2] = input[1];
		output_v[output_column / 2] = input[3];
	}
	else
// Store Y and advance output for odd pixels only
	{
		output_y[output_column] = input[2];
	}
}

static inline void transfer_YUV422_to_YUV420P(unsigned char *output_y, 
	unsigned char *output_u, 
	unsigned char *output_v, 
	unsigned char *input,
	int output_column,
	int output_row)
{
// Store U and V for even pixels only
	if(!(output_column & 1) && !(output_row & 1))
	{
		output_y[output_column] = input[0];
		output_u[output_column / 2] = input[1];
		output_v[output_column / 2] = input[3];
	}
	else
// Store Y and advance output for odd pixels only
	{
		output_y[output_column] = input[2];
	}
}










#define TRANSFER_FRAME_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		unsigned char *input_row = input_rows[row_table[i]]; \
		int bit_counter = 7; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_FRAME_TAIL \
		} \
	}

#define TRANSFER_YUV420P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = out_y_plane + row_table[i] * total_out_w + out_x; \
		unsigned char *output_u = out_u_plane + i / 2 * total_out_w / 2 + out_x / 2; \
		unsigned char *output_v = out_v_plane + i / 2 * total_out_w / 2 + out_x / 2; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV422P_OUT_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *input_row = input_rows[row_table[i]]; \
		unsigned char *output_y = out_y_plane + row_table[i] * total_out_w + out_x; \
		unsigned char *output_u = out_u_plane + i * total_out_w / 2 + out_x / 2; \
		unsigned char *output_v = out_v_plane + i * total_out_w / 2 + out_x / 2; \
		for(j = 0; j < out_w; j++) \
		{

#define TRANSFER_YUV420P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		unsigned char *input_y = in_y_plane + row_table[i] * total_in_w; \
		unsigned char *input_u = in_u_plane + (row_table[i] / 2) * (total_in_w / 2); \
		unsigned char *input_v = in_v_plane + (row_table[i] / 2) * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV422P_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i + out_y] + out_x * out_pixelsize; \
		unsigned char *input_y = in_y_plane + row_table[i] * total_in_w; \
		unsigned char *input_u = in_u_plane + row_table[i] * (total_in_w / 2); \
		unsigned char *input_v = in_v_plane + row_table[i] * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{


#define TRANSFER_YUV422_IN_HEAD \
	for(i = 0; i < out_h; i++) \
	{ \
		unsigned char *output_row = output_rows[i + out_y] + ((out_x * out_pixelsize) & 0xfffffffc); \
		unsigned char *input_y = in_y_plane + row_table[i] * total_in_w; \
		unsigned char *input_u = in_u_plane + row_table[i] * (total_in_w / 2); \
		unsigned char *input_v = in_v_plane + row_table[i] * (total_in_w / 2); \
		for(j = 0; j < out_w; j++) \
		{







#define TRANSFER_FRAME_PERMUTATION2(output, \
	input, \
	y_in_offset, \
	u_in_offset, \
	v_in_offset, \
	input_column) \
{ \
	register int i, j; \
	switch(in_colormodel) \
	{ \
		case BC_YUV420P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV420P_to_RGB8((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV420P_to_RGB565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV420P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV420P_to_BGR888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV420P_to_BGR8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_YUV420P_IN_HEAD \
					transfer_YUV422P_to_YUV888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_YUV422P: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV420P_to_RGB8((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV420P_to_RGB565((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV420P_to_RGB888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV420P_to_BGR888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV420P_to_BGR8888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888: \
					TRANSFER_YUV422P_IN_HEAD \
					transfer_YUV422P_to_YUV888((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422: \
					TRANSFER_YUV422_IN_HEAD \
					transfer_YUV422P_to_YUV422((output), \
						input_y + (y_in_offset), \
						input_u + (u_in_offset), \
						input_v + (v_in_offset), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_YUV422: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_RGB8((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_RGB565((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB888:      \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_RGB888((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV888:      \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_YUV888((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_BGR888((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_YUV422_to_BGR8888((output), (input), (input_column)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_YUV422_to_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_YUV422_to_YUV420P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j, \
						i); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGBA16161616: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB8((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_RGB565((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888:      \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR888((output), (input)); \
					TRANSFER_FRAME_TAIL \
				break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA16161616_to_BGR8888((output), (input)); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGBA8888: \
			switch(out_colormodel) \
			{ \
				case BC_TRANSPARENCY: \
					TRANSFER_FRAME_HEAD \
					transfer_RGBA8888_to_TRANSPARENCY((output), (input), &bit_counter); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_RGB8: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB8bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB8((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB565bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_RGB565((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR888bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					if(bg_color > 0) \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR8888bg((output), (input), bg_r, bg_g, bg_b); \
						TRANSFER_FRAME_TAIL \
					else \
						TRANSFER_FRAME_HEAD \
						transfer_RGBA8888_to_BGR8888((output), (input)); \
						TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_RGB888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB8: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB8((output), (input));      \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR565: \
				case BC_RGB565: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_RGB565((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_BGR8888: \
					TRANSFER_FRAME_HEAD \
					transfer_RGB888_to_BGR8888((output), (input));   \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV420P: \
					TRANSFER_YUV420P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
				case BC_YUV422P: \
					TRANSFER_YUV422P_OUT_HEAD \
					transfer_RGB888_to_YUV420P_YUV422P(output_y, \
						output_u, \
						output_v, \
						(input), \
						j); \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_BGR8888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR8888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
 \
		case BC_BGR888: \
			switch(out_colormodel) \
			{ \
				case BC_RGB888: \
					TRANSFER_FRAME_HEAD \
					transfer_BGR888_to_RGB888((output), (input));    \
					TRANSFER_FRAME_TAIL \
					break; \
			} \
			break; \
	} \
}

static void get_scale_tables(int **column_table, 
	int **row_table, 
	int in_x1, 
	int in_y1, 
	int in_x2, 
	int in_y2,
	int out_x1, 
	int out_y1, 
	int out_x2, 
	int out_y2)
{
	int y_out, i;
	float w_in = in_x2 - in_x1;
	float h_in = in_y2 - in_y1;
	int w_out = out_x2 - out_x1;
	int h_out = out_y2 - out_y1;

	float hscale = w_in / w_out;
	float vscale = h_in / h_out;

	(*column_table) = malloc(sizeof(int) * w_out);
	(*row_table) = malloc(sizeof(int) * h_out);
	for(i = 0; i < w_out; i++)
	{
		(*column_table)[i] = (int)(hscale * i) + in_x1;
	}

	for(i = 0; i < h_out; i++)
	{
		(*row_table)[i] = (int)(vscale * i) + in_y1;
//printf("get_scale_tables %d %d\n", i, (int)(vscale * i));
	}
}

void cmodel_transfer(unsigned char **output_rows, 
	unsigned char **input_rows,
	unsigned char *out_y_plane,
	unsigned char *out_u_plane,
	unsigned char *out_v_plane,
	unsigned char *in_y_plane,
	unsigned char *in_u_plane,
	unsigned char *in_v_plane,
	int in_x, 
	int in_y, 
	int in_w, 
	int in_h,
	int out_x, 
	int out_y, 
	int out_w, 
	int out_h,
	int in_colormodel, 
	int out_colormodel,
	int bg_color,
	int total_in_w,
	int total_out_w)
{
	int *column_table;
	int *row_table;
	int scale_x, scale;
	int bg_r, bg_g, bg_b;
	int in_pixelsize = cmodel_calculate_pixelsize(in_colormodel);
	int out_pixelsize = cmodel_calculate_pixelsize(out_colormodel);

	bg_r = (bg_color & 0xff0000) >> 16;
	bg_g = (bg_color & 0xff00) >> 8;
	bg_b = (bg_color & 0xff);

// Initialize tables
	if(yuv_table == 0)
	{
		yuv_table = calloc(1, sizeof(cmodel_yuv_t));
		cmodel_init_yuv(yuv_table);
	}

// Get scaling
	scale = scale_x = (out_w != in_w);
	get_scale_tables(&column_table, &row_table, 
		in_x, in_y, in_x + in_w, in_y + in_h,
		out_x, out_y, out_x + out_w, out_y + out_h);

//printf("cmodel_transfer %d %d\n", in_colormodel, out_colormodel);

	switch(in_colormodel)
	{
		case BC_YUV420P:
		case BC_YUV422P:
			if(scale)
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + column_table[j] * in_pixelsize,
					column_table[j],
					column_table[j] / 2,
					column_table[j] / 2,
					0);
			}
			else
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + j * in_pixelsize,
					j,
					j / 2,
					j / 2,
					0);
			}
			break;

		case BC_YUV422:
			if(scale)
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + ((column_table[j] * in_pixelsize) & 0xfffffffc),
					0,
					0,
					0,
					column_table[j]);
			}
			else
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + ((j * in_pixelsize) & 0xfffffffc),
					0,
					0,
					0,
					j);
			}
			break;

		default:
			if(scale)
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + column_table[j] * in_pixelsize,
					0,
					0,
					0,
					0);
			}
			else
			{
				TRANSFER_FRAME_PERMUTATION2(&output_row, 
					input_row + j * in_pixelsize,
					0,
					0,
					0,
					0);
			}
			break;
	}

	free(column_table);
	free(row_table);
}

int cmodel_bc_to_x(int color_model)
{
	switch(color_model)
	{
		case BC_YUV420P:
			return FOURCC_YV12;
			break;
		case BC_YUV422:
			return FOURCC_YUV2;
			break;
	}
	return -1;
}

void cmodel_to_text(char *string, int cmodel)
{
	switch(cmodel)
	{
		case BC_RGB888:       strcpy(string, "RGB-8 Bit");   break;
		case BC_RGBA8888:     strcpy(string, "RGBA-8 Bit");  break;
		case BC_RGB161616:    strcpy(string, "RGB-16 Bit");  break;
		case BC_RGBA16161616: strcpy(string, "RGBA-16 Bit"); break;
		case BC_YUV888:       strcpy(string, "YUV-8 Bit");   break;
		case BC_YUVA8888:     strcpy(string, "YUVA-8 Bit");  break;
		case BC_YUV161616:    strcpy(string, "YUV-16 Bit");  break;
		case BC_YUVA16161616: strcpy(string, "YUVA-16 Bit"); break;
		default: strcpy(string, "RGB-8 Bit"); break;
	}
}

int cmodel_from_text(char *text)
{
	if(!strcasecmp(text, "RGB-8 Bit"))   return BC_RGB888;
	if(!strcasecmp(text, "RGBA-8 Bit"))  return BC_RGBA8888;
	if(!strcasecmp(text, "RGB-16 Bit"))  return BC_RGB161616;
	if(!strcasecmp(text, "RGBA-16 Bit")) return BC_RGBA16161616;
	if(!strcasecmp(text, "YUV-8 Bit"))   return BC_YUV888;
	if(!strcasecmp(text, "YUVA-8 Bit"))  return BC_YUVA8888;
	if(!strcasecmp(text, "YUV-16 Bit"))  return BC_YUV161616;
	if(!strcasecmp(text, "YUVA-16 Bit")) return BC_YUVA16161616;
	return BC_RGB888;
}

