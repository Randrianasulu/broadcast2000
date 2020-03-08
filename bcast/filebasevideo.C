#include <string.h>
#include "assets.h"
#include "byteorder.h"
#include "file.h"
#include "filebase.h"
#include "overlayframe.h"
#include "pluginbuffer.h"
#include "sizes.h"
#include "vframe.h"

// ===================================== all video codecs

int FileBase::get_bytes_per_pixel(int color_model)
{
	switch(color_model)
	{
		case FILEBASE_GREY:
			return 1;
			break;
		case FILEBASE_YUV:
			return 2;
			break;
		case FILEBASE_RAW:
			return 3;
			break;
		case FILEBASE_RGBA:
			return 4;
			break;
	}
}


int FileBase::transfer_from(VFrame *frame_out, VFrame *frame_in, 
		float in_x1, float in_y1, float in_x2, float in_y2,
		float out_x1, float out_y1, float out_x2, float out_y2, 
		int alpha, int use_alpha, int use_float, int interpolate, 
		int mode)
{
	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, interpolate, mode) ||
			overlayer->color_model != OVERLAY_VFRAME)
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, interpolate, mode, OVERLAY_VFRAME);
	}

	overlayer->overlay(frame_out, frame_in,
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha);
}

// alpha is from 0 to VMAX for frame operations

long FileBase::raw_to_frame(unsigned char *in_buffer,
					VFrame *out_frame, 
					float in_x1, float in_y1, float in_x2, float in_y2,
					float out_x1, float out_y1, float out_x2, float out_y2, 
					int alpha, int use_alpha, int use_float, int color_model, 
					int interpolate, int in_w, int in_h)
{
	if(overlayer)
	{
		if(!overlayer->compare_with(use_float, use_alpha, interpolate, NORMAL) ||
			overlayer->color_model != color_model)
		{
			delete overlayer;
			overlayer = 0;
		}
	}

	if(!overlayer)
	{
		overlayer = new OverlayFrame(use_alpha, use_float, interpolate, NORMAL, color_model);
	}

	overlayer->overlay(out_frame, in_buffer,
		in_x1, in_y1, in_x2, in_y2,
		out_x1, out_y1, out_x2, out_y2, 
		alpha, in_w, in_h);

	return 0;
}


long FileBase::raw_to_frame(unsigned char *in_buffer,
		VFrame *out_frame, 
		int in_x1, int in_y1, int in_x2, int in_y2,
		int out_x1, int out_y1, int out_x2, int out_y2, 
		int alpha, int use_alpha, int use_float, int color_model)
{
	int bytes_per_pixel = get_bytes_per_pixel(color_model);

	if(in_x2 - in_x1 == out_x2 - out_x1 && 
		in_y2 - in_y1 == out_y2 - out_y1)
	{
// direct transfer
		for( ; in_y1 < in_y2; in_y1++)
		{
			raw_to_row_direct(&in_buffer[in_y1 * asset->width * bytes_per_pixel + in_x1 * bytes_per_pixel], 
				&(((VPixel**)out_frame->get_rows())[out_y1][out_x1]), out_x2 - out_x1, 
				alpha, alpha < VMAX ? use_alpha : 0, use_float, color_model);
			out_y1++;
		}
	}
	else
	{
// scale transfer
		int y_out, h_out = out_y2 - out_y1;
		int *column_table, *row_table;

		column_table = new int[out_x2 - out_x1];
		row_table = new int[h_out];

		out_frame->get_scale_tables(column_table, row_table, 
			in_x1, in_y1, in_x2, in_y2,
			out_x1, out_y1, out_x2, out_y2);

//printf("FileBase::raw_to_frame scale transfer h_out %d\n", h_out);
		for(y_out = 0; y_out < h_out; y_out++)
		{
			raw_to_row_scale(&in_buffer[row_table[y_out] * asset->width * bytes_per_pixel + in_x1 * bytes_per_pixel], 
				&(((VPixel**)out_frame->get_rows())[out_y1][out_x1]), out_x2 - out_x1, column_table, 
				alpha, alpha < VMAX ? use_alpha : 0, use_float, color_model);
			out_y1++;
		}
		delete [] column_table;
		delete [] row_table;
	}
	return 0;
}

// alpha is from 0 to 255 or a float from 0 to 1 for row operations

#define ROW_LOOP_BEGIN \
	while(out_row < row_end) \
	{

#define ALPHA_SWITCH \
	switch(color_model) \
	{ \
		case FILEBASE_GREY: \
			ROW_LOOP_BEGIN \
				grey_to_pixel_alpha(in_buffer++, out_row++, a, output_opacity); \
			} \
			break; \
		case FILEBASE_YUV: \
			ROW_LOOP_BEGIN \
				yuv_to_pixel_alpha(in_buffer, out_row, 3, a, output_opacity); \
				in_buffer += 4; \
				out_row += 2; \
			} \
			break; \
		case FILEBASE_RAW: \
			ROW_LOOP_BEGIN \
				raw_to_pixel_alpha(in_buffer, out_row++, a, output_opacity); \
				in_buffer += 3; \
			} \
			break; \
	}

int FileBase::raw_to_row_direct(unsigned char *in_buffer, VPixel *out_row, 
	int w, int alpha, int use_alpha, int use_float, int color_model)
{
	VPixel *row_end = out_row + w;

	if(use_alpha)
	{
		if(use_float)
		{
			float a = (float)alpha / VMAX;
			float output_opacity = 1 - a;
			
			ALPHA_SWITCH
		}
		else
		{
			int a = alpha;
			int output_opacity = VMAX - a;
#if (VMAX == 65535)
			a >>= 8;
			output_opacity >>= 8;
#endif

			ALPHA_SWITCH
		}
	}
	else
	switch(color_model)
	{
		case FILEBASE_GREY:
			ROW_LOOP_BEGIN
				grey_to_pixel(in_buffer++, out_row++);
			}
			break;
		case FILEBASE_YUV:
			if(use_float)
			{
				ROW_LOOP_BEGIN
					yuv_to_pixel_float(in_buffer, out_row, 3);
					in_buffer += 4;
					out_row += 2;
				}
			}
			else
			{
				ROW_LOOP_BEGIN
					yuv_to_pixel_int(in_buffer, out_row, 3);
					in_buffer += 4;
					out_row += 2;
				}
			}
			break;
		case FILEBASE_RAW:
			ROW_LOOP_BEGIN
				raw_to_pixel(in_buffer, out_row++);
				in_buffer += 3;
			}
			break;
	}
}

#define SCALE_ROW_LOOP_BEGIN \
			for(int i = 0; i < w; ) \
			{

#define SCALE_ALPHA_SWITCH \
	switch(color_model) \
	{ \
		case FILEBASE_GREY: \
			SCALE_ROW_LOOP_BEGIN \
				grey_to_pixel_alpha(&input[column_table[i]], &output[i], a, output_opacity); \
				i++; \
			} \
			break; \
		case FILEBASE_YUV: \
			SCALE_ROW_LOOP_BEGIN \
				in_column = column_table[i]; \
				yuv_column = in_column / 2 * 4; \
				yuv_to_pixel_alpha(&input[yuv_column], &output[i], (yuv_column < in_column * 2) ? 2 : 1, a, output_opacity); \
				i++; \
			} \
			break; \
		case FILEBASE_RAW: \
			SCALE_ROW_LOOP_BEGIN \
				raw_to_pixel_alpha(&input[column_table[i] * 3], &output[i], a, output_opacity); \
				i++; \
			} \
			break; \
	}


int FileBase::raw_to_row_scale(unsigned char *input, VPixel *output, 
	int w, int *column_table, int alpha, int use_alpha, int use_float, int color_model)
{
	int in_column, yuv_column;
	if(use_alpha)
	{
		if(use_float)
		{
			float a = (float)alpha / VMAX;
			float output_opacity = 1 - a;

			SCALE_ALPHA_SWITCH
		}
		else
		{
			int a = alpha;
			int output_opacity = VMAX - a;
#if (VMAX == 65535)
			a >>= 8;
			output_opacity >>= 8;
#endif

			SCALE_ALPHA_SWITCH
		}
	}
	else
	{
		switch(color_model)
		{
			case FILEBASE_GREY:
				SCALE_ROW_LOOP_BEGIN
					grey_to_pixel(&input[column_table[i]], &output[i]);
					i++;
				}
				break;
			case FILEBASE_YUV:
				if(use_float)
				{
					SCALE_ROW_LOOP_BEGIN
						in_column = column_table[i];
						yuv_column = in_column / 2 * 4;
						yuv_to_pixel_float(&input[yuv_column], &output[i], (yuv_column < in_column * 2) ? 2 : 1);
						i++;
					}
				}
				else
				{
					SCALE_ROW_LOOP_BEGIN
						in_column = column_table[i];
						yuv_column = in_column / 2 * 4;
						yuv_to_pixel_int(&input[yuv_column], &output[i], (yuv_column < in_column * 2) ? 2 : 1);
						i++;
					}
				}
				break;
			case FILEBASE_RAW:
				SCALE_ROW_LOOP_BEGIN
					raw_to_pixel(&input[column_table[i] * 3], &output[i]);
					i++;
				}
				break;
		}
	}
}



#define ALPHA_OUTPUT_FLOAT \
	output->a = VMAX; \
	output->r = (VWORD)(output_opacity * output->r); \
	output->g = (VWORD)(output_opacity * output->g); \
	output->b = (VWORD)(output_opacity * output->b);


int FileBase::raw_to_pixel_alpha(unsigned char *input, VPixel *output, float a, float output_opacity)
{
	ALPHA_OUTPUT_FLOAT

#if (VMAX == 65535)
	output->r += (VWORD)(a * *input++) << 8;
	output->g += (VWORD)(a * *input++) << 8;
	output->b += (VWORD)(a * *input) << 8;
#else
	output->r += (VWORD)(a * *input++);
	output->g += (VWORD)(a * *input++);
	output->b += (VWORD)(a * *input);
#endif
return 0;
}

#define ALPHA_OUTPUT_INT \
	output->a = VMAX; \
	output->r = (VWORD)((output_opacity * output->r) >> 8); \
	output->g = (VWORD)((output_opacity * output->g) >> 8); \
	output->b = (VWORD)((output_opacity * output->b) >> 8);

int FileBase::raw_to_pixel_alpha(unsigned char *input, VPixel *output, int a, int output_opacity)
{
	ALPHA_OUTPUT_INT

#if (VMAX == 65535)
	output->r += (VWORD)(a * *input++);
	output->g += (VWORD)(a * *input++);
	output->b += (VWORD)(a * *input);
#else
	output->r += (VWORD)((a * *input++) >> 8);
	output->g += (VWORD)((a * *input++) >> 8);
	output->b += (VWORD)((a * *input) >> 8);
#endif
return 0;
}

int FileBase::raw_to_pixel(unsigned char *input, VPixel *output)
{
	output->a = VMAX;
	output->r = input[0];
	output->g = input[1];
	output->b = input[2];

#if (VMAX == 65535)
	output->r <<= 8;
	output->g <<= 8;
	output->b <<= 8;
#endif
return 0;
}

// converts 1 byte to 1 pixel
int FileBase::grey_to_pixel_alpha(unsigned char *input, VPixel *output, float a, float output_opacity)
{
	static VWORD result;
	ALPHA_OUTPUT_FLOAT

	result = (VWORD)(a * *input);

#if (VMAX == 65535)
	result <<= 8;
#endif

	output->r += result;
	output->g += result;
	output->b += result;
return 0;
}

int FileBase::grey_to_pixel_alpha(unsigned char *input, VPixel *output, int a, int output_opacity)
{
	static VWORD result;
	ALPHA_OUTPUT_INT


#if (VMAX == 65535)
	result = (VWORD)(a * *input);
#else
	result = (VWORD)((a * *input) >> 8);
#endif

	output->r += result;
	output->g += result;
	output->b += result;
return 0;
}

int FileBase::grey_to_pixel(unsigned char *input, VPixel *output)
{
	output->a = VMAX;
#if (VMAX == 65535)
	output->r = output->g = output->b = (VWORD)*input << 8;
#else
	output->r = output->g = output->b = *input;
#endif
return 0;
}

#define TEST_BOUNDARY \
	if(r > VMAX) r = VMAX; \
	if(g > VMAX) g = VMAX; \
	if(b > VMAX) b = VMAX; \
	if(r < 0) r = 0; \
	if(g < 0) g = 0; \
	if(b < 0) b = 0;


#define STORE_RGB \
	TEST_BOUNDARY \
	output->r += (VWORD)r; \
	output->g += (VWORD)g; \
	output->b += (VWORD)b;

#define REPLACE_RGB \
	TEST_BOUNDARY \
	output->r = (VWORD)r; \
	output->g = (VWORD)g; \
	output->b = (VWORD)b;

// converts 4 bytes to 2 pixels
int FileBase::yuv_to_pixel_alpha(unsigned char *input, VPixel *output, int pixel, float a, float output_opacity)
{
	static float y1, u, v, y2, r, g, b;
	u = (float)(input[1]);
	v = (float)(input[3]);

	if(pixel & 0x01)
	{
		ALPHA_OUTPUT_FLOAT
		y1 = (float)(input[0]);
		r = y1 + (float)1.140 * v;
		g = y1 - (float).396 * u - (float).581 * v;
		b = y1 + (float)2.029 * u;

#if (VMAX == 65535)
		r *= 256;
		g *= 256;
		b *= 256;
#endif

		STORE_RGB
		output++;
	}

	if(pixel & 0x02)
	{
		ALPHA_OUTPUT_FLOAT
		y2 = (float)(input[2]);
		r = y2 + (float)1.140 * v;
		g = y2 - (float).396 * u - (float).581 * v;
		b = y2 + (float)2.029 * u;
	
#if (VMAX == 65535)
		r *= 256;
		g *= 256;
		b *= 256;
#endif

		STORE_RGB
	}
return 0;
}

// converts 4 bytes to 2 pixels
int FileBase::yuv_to_pixel_alpha(unsigned char *input, VPixel *output, int pixel, int a, int output_opacity)
{
	static int y1, u, v, y2, r, g, b;

	u = (int)(input[1]);
	v = (int)(input[3]);

	if(pixel & 0x01)
	{
		ALPHA_OUTPUT_INT
		y1 = (int)(input[0]);
		r = 256 * y1 + 292 * v;
		g = 256 * y1 - 101 * u - 149 * v;
		b = 256 * y1 + 564 * u;

#if (VMAX == 255)
		r >>= 8;
		g >>= 8;
		b >>= 8;
#endif

		STORE_RGB
		output++;
	}

	if(pixel & 0x02)
	{
		ALPHA_OUTPUT_INT
		y2 = (int)(input[2]);
		r = 256 * y2 + 292 * v;
		g = 256 * y2 - 101 * u - 149 * v;
		b = 256 * y2 + 564 * u;
	
#if (VMAX == 65535)
		r >>= 8;
		g >>= 8;
		b >>= 8;
#endif

		STORE_RGB
	}
return 0;
}


// converts 4 bytes to 2 pixels
int FileBase::yuv_to_pixel_float(unsigned char *input, VPixel *output, int pixel)
{
	static float y1, u, v, y2, r, g, b;

	u = (float)(input[1]);
	v = (float)(input[3]);

	if(pixel & 0x01)
	{
		output->a = VMAX;
		y1 = (float)(input[0]);
		r = y1 + (float)1.402 * v;
		g = y1 - (float).344 * u - (float).714 * v;
		b = y1 + (float)1.772 * u;

#if (VMAX == 65535)
		r *= 256;
		g *= 256;
		b *= 256;
#endif

		REPLACE_RGB
		output++;
	}

	if(pixel & 0x02)
	{
		output->a = VMAX;
		y2 = (float)(input[2]);
		r = y2 + (float)1.402 * v;
		g = y2 - (float).344 * u - (float).714 * v;
		b = y2 + (float)1.772 * u;

#if (VMAX == 65535)
		r *= 256;
		g *= 256;
		b *= 256;
#endif

		REPLACE_RGB
	}
return 0;
}


// converts 4 bytes to 2 pixels
int FileBase::yuv_to_pixel_int(unsigned char *input, VPixel *output, int pixel)
{
	static int y1, u, v, y2, r, g, b;

	u = (int)(input[1]);
	v = (int)(input[3]);

	if(pixel & 0x01)
	{
		output->a = VMAX;
		y1 = (int)(input[0]);
		r = 256 * y1 + 292 * v;
		g = 256 * y1 - 101 * u - 149 * v;
		b = 256 * y1 + 564 * u;

#if (VMAX == 255)
		r >>= 8;
		g >>= 8;
		b >>= 8;
#endif

		REPLACE_RGB
		output++;
	}

	if(pixel & 0x02)
	{
		output->a = VMAX;
		y2 = (int)(input[2]);
		r = 256 * y2 + 292 * v;
		g = 256 * y2 - 101 * u - 149 * v;
		b = 256 * y2 + 564 * u;

#if (VMAX == 65535)
		r >>= 8;
		g >>= 8;
		b >>= 8;
#endif

		REPLACE_RGB
	}
return 0;
}








long FileBase::frame_to_raw(unsigned char *out_buffer,
					VFrame *in_frame,
					int w,
					int h,
					int use_alpha,
					int use_float,
					int color_model)
{
	int bytes_per_pixel = get_bytes_per_pixel(color_model);

	for(int y = 0; y < h; y++)
	{
		row_to_raw(&out_buffer[y * w * bytes_per_pixel], 
			((VPixel**)in_frame->get_rows())[y], w, use_alpha, use_float, color_model);
	}
	return (long)h * w * bytes_per_pixel;
}

int FileBase::row_to_raw(unsigned char *out_buffer, VPixel *in_row, 
	int w, int use_alpha, int use_float, int color_model)
{
	if(use_alpha)
	{
		if(use_float)
		{
			switch(color_model)
			{
				case FILEBASE_YUV:
					for(int x = 0; x < w; x += 2)
					{
						pixel_to_yuv_alpha_float(&in_row[x], out_buffer);
						out_buffer += 4;
					}
					break;
				case FILEBASE_RAW:
					for(int x = 0; x < w; x++)
					{
						pixel_to_raw_alpha_float(&in_row[x], out_buffer);
						out_buffer += 3;
					}
					break;
				case FILEBASE_RGBA:
					for(int x = 0; x < w; x++)
					{
						pixel_to_rgba_alpha_float(&in_row[x], out_buffer);
						out_buffer += 4;
					}
					break;
			}
		}
		else
		{
			switch(color_model)
			{
				case FILEBASE_YUV:
					for(int x = 0; x < w; x += 2)
					{
						pixel_to_yuv_alpha_int(&in_row[x], out_buffer);
						out_buffer += 4;
					}
					break;
				case FILEBASE_RAW:
					for(int x = 0; x < w; x++)
					{
						pixel_to_raw_alpha_int(&in_row[x], out_buffer);
						out_buffer += 3;
					}
				case FILEBASE_RGBA:
					for(int x = 0; x < w; x++)
					{
						pixel_to_rgba_alpha_int(&in_row[x], out_buffer);
						out_buffer += 4;
					}
					break;
			}
		}
	}
	else
	{
		switch(color_model)
		{
			case FILEBASE_YUV:
				if(use_float)
				{
					for(int x = 0; x < w; x += 2)
					{
						pixel_to_yuv_float(&in_row[x], out_buffer);
						out_buffer += 4;
					}
				}
				else
				{
					for(int x = 0; x < w; x += 2)
					{
						pixel_to_yuv_int(&in_row[x], out_buffer);
						out_buffer += 4;
					}
				}
				break;
			case FILEBASE_RAW:
				for(int x = 0; x < w; x++)
				{
					pixel_to_raw(&in_row[x], out_buffer);
					out_buffer += 3;
				}
			case FILEBASE_RGBA:
				for(int x = 0; x < w; x++)
				{
					pixel_to_rgba(&in_row[x], out_buffer);
					out_buffer += 4;
				}
				break;
		}
	}
return 0;
}

int FileBase::pixel_to_raw_alpha_float(VPixel *input, unsigned char *output)
{
	static float r1, g1, b1, a1;

	a1 = (float)input->a / VMAX;
	r1 = a1 * input->r;
	g1 = a1 * input->g;
	b1 = a1 * input->b;

#if (VMAX == 65535)     // scale down to unsigned char
	*output++ = (unsigned char)(r1 / 256);
	*output++ = (unsigned char)(g1 / 256);
	*output = (unsigned char)(b1 / 256);
#else
	*output++ = (unsigned char)r1;
	*output++ = (unsigned char)g1;
	*output = (unsigned char)b1;
#endif
return 0;
}

int FileBase::pixel_to_raw_alpha_int(VPixel *input, unsigned char *output)
{
	static int r1, g1, b1, a1;

	a1 = input->a;
#if (VMAX == 65535)
	a1 >>= 8;
#endif
	r1 = a1 * input->r;
	g1 = a1 * input->g;
	b1 = a1 * input->b;

#if (VMAX == 65535)    // scale down to unsigned char
	*output++ = (unsigned char)(r1 >> 16);
	*output++ = (unsigned char)(g1 >> 16);
	*output++ = (unsigned char)(b1 >> 16);
#else
	*output++ = (unsigned char)(r1 >> 8);
	*output++ = (unsigned char)(g1 >> 8);
	*output++ = (unsigned char)(b1 >> 8);
#endif
return 0;
}


int FileBase::pixel_to_raw(VPixel *input, unsigned char *output)
{
#if (VMAX == 65535)
	*output++ = (unsigned char)(input->r >> 8);
	*output++ = (unsigned char)(input->g >> 8);
	*output++ = (unsigned char)(input->b >> 8);
#else
	*output++ = input->r;
	*output++ = input->g;
	*output++ = input->b;
#endif
return 0;
}

int FileBase::pixel_to_rgba_alpha_float(VPixel *input, unsigned char *output)
{
#if (VMAX == 65535)     // scale down to unsigned char
	*output++ = (unsigned char)(input->r >> 8);
	*output++ = (unsigned char)(input->g >> 8);
	*output++ = (unsigned char)(input->b >> 8);
	*output = (unsigned char)(input->a >> 8);
#else
	*output++ = (unsigned char)(input->r);
	*output++ = (unsigned char)(input->g);
	*output++ = (unsigned char)(input->b);
	*output = (unsigned char)(input->a);
#endif
return 0;
}

int FileBase::pixel_to_rgba_alpha_int(VPixel *input, unsigned char *output)
{
#if (VMAX == 65535)     // scale down to unsigned char
	*output++ = (unsigned char)(input->r >> 8);
	*output++ = (unsigned char)(input->g >> 8);
	*output++ = (unsigned char)(input->b >> 8);
	*output = (unsigned char)(input->a >> 8);
#else
	*output++ = (unsigned char)(input->r);
	*output++ = (unsigned char)(input->g);
	*output++ = (unsigned char)(input->b);
	*output = (unsigned char)(input->a);
#endif
return 0;
}


int FileBase::pixel_to_rgba(VPixel *input, unsigned char *output)
{
#if (VMAX == 65535)     // scale down to unsigned char
	*output++ = (unsigned char)(input->r >> 8);
	*output++ = (unsigned char)(input->g >> 8);
	*output++ = (unsigned char)(input->b >> 8);
	*output = (unsigned char)(input->a >> 8);
#else
	*output++ = (unsigned char)(input->r);
	*output++ = (unsigned char)(input->g);
	*output++ = (unsigned char)(input->b);
	*output = (unsigned char)(input->a);
#endif
return 0;
}

#if (VMAX == 65535)
#define STORE_YUV_FLOAT \
	*output++ = (unsigned char)(y1 / 256); \
	*output++ = (unsigned char)(u / 256); \
	*output++ = (unsigned char)(y2 / 256); \
	*output++ = (unsigned char)(v / 256);
#else
#define STORE_YUV_FLOAT \
	*output++ = (unsigned char)y1; \
	*output++ = (unsigned char)u; \
	*output++ = (unsigned char)y2; \
	*output++ = (unsigned char)v;
#endif

#define DO_YUV_FLOAT \
	y1 = (float).299 * r1 + (float).587 * g1 + (float).114 * b1; \
	y2 = (float).299 * r2 + (float).587 * g2 + (float).114 * b2; \
	u = (float)-.147 / 2 * (r1 + r2) - (float).289 / 2 * (g1 + g2) + (float).436 / 2 * (b1 + b2); \
	v = (float).615 / 2 * (r1 + r2) - (float).515 / 2 * (g1 + g2) - (float).1 / 2 * (b1 + b2); \
 \
	if(u < 0) { u = 0; } \
	else \
	if(v < 0) { v = 0; } \
	if(u > VMAX) u = VMAX; \
	if(v > VMAX) v = VMAX; \
 \
 	STORE_YUV_FLOAT


int FileBase::pixel_to_yuv_alpha_float(VPixel *input, unsigned char *output)
{
	static float r1, g1, b1, a1, r2, g2, b2, a2;
	static float y1, u, v, y2;

	a1 = (float)input->a / VMAX;
	r1 = a1 * input->r;
	g1 = a1 * input->g;
	b1 = a1 * input->b;

	input++;
	a2 = (float)input->a / VMAX;
	r2 = a2 * input->r;
	g2 = a2 * input->g;
	b2 = a2 * input->b;

	DO_YUV_FLOAT
return 0;
}

#if (VMAX == 65535)
#define STORE_YUV_INT \
	y1 >>= 16; \
	u >>= 16; \
	y2 >>= 16; \
	v >>= 16;
#else
#define STORE_YUV_INT \
	y1 >>= 8; \
	u >>= 8; \
	y2 >>= 8; \
	v >>= 8;
#endif

#define DO_YUV_INT \
	y1 = 77 * r1 + 150 * g1 + 29 * b1; \
	y2 = 77 * r2 + 150 * g2 + 29 * b2; \
	u = -38 / 2 * (r1 + r2) - 74 / 2 * (g1 + g2) + 112 / 2 * (b1 + b2); \
	v = 158 / 2 * (r1 + r2) - 132 / 2 * (g1 + g2) - 26 / 2 * (b1 + b2); \
 \
 	STORE_YUV_INT \
	if(u < 0) { u = 0; } \
	else \
	if(v < 0) { v = 0; } \
	if(u > 255) u = 255; \
	if(v > 255) v = 255; \
 \
	*output++ = (unsigned char)(y1); \
	*output++ = (unsigned char)(u); \
	*output++ = (unsigned char)(y2); \
	*output++ = (unsigned char)(v);


int FileBase::pixel_to_yuv_alpha_int(VPixel *input, unsigned char *output)
{
	static int r1, g1, b1, a1, r2, g2, b2, a2;
	static int y1, u, v, y2;

	a1 = input->a;
#if (VMAX == 65535)
	a1 >>= 8;
#endif
	r1 = a1 * input->r;
	g1 = a1 * input->g;
	b1 = a1 * input->b;

	r1 >>= 8;
	g1 >>= 8;
	b1 >>= 8;

	input++;
	a2 = input->a;
#if (VMAX == 65535)
	a2 >>= 8;
#endif
	r2 = a2 * input->r;
	g2 = a2 * input->g;
	b2 = a2 * input->b;

	r2 >>= 8;
	g2 >>= 8;
	b2 >>= 8;

	DO_YUV_INT
return 0;
}

int FileBase::pixel_to_yuv_float(VPixel *input, unsigned char *output)
{
	static float r1, g1, b1, r2, g2, b2;
	static float y1, u, v, y2;

	r1 = input->r;
	g1 = input->g;
	b1 = input->b;

	input++;
	r2 = input->r;
	g2 = input->g;
	b2 = input->b;
	
	DO_YUV_FLOAT
return 0;
}

int FileBase::pixel_to_yuv_int(VPixel *input, unsigned char *output)
{
	static int r1, g1, b1, r2, g2, b2;
	static int y1, u, v, y2;

	r1 = input->r;
	g1 = input->g;
	b1 = input->b;

	input++;
	r2 = input->r;
	g2 = input->g;
	b2 = input->b;

	DO_YUV_INT
return 0;
}

