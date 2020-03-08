#ifndef BCBITMAP_H
#define BCBITMAP_H

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

#include "bctool.inc"
#include "bcwindow.inc"
#include "sizes.h"
#include "vframe.inc"

#define FOURCC_YV12 0x32315659  /* YV12 YUV 4:2:0 */

class BC_Bitmap
{
public:
	BC_Bitmap(BC_Window *top_level, int w, int h, int depth);
	virtual ~BC_Bitmap();

// transfer VFrame with alpha
	int read_frame(VFrame *frame, int x1, int y1, int x2, int y2, int use_alpha);
// transfer VFRAME_RGB888
	int read_frame(unsigned char **rows, int in_w, int in_h);


// If dont_wait is true, the XSync call comes before the flash.
// For YUV bitmaps, the image is scaled to fill dest_x ... w * dest_y ... h
	int write_drawable(Drawable &pixmap, int dest_x, int dest_y, int source_x, int source_y, int w, int h, int dont_wait);
// the bitmap must be wholly contained in the source during a GetImage
	int read_drawable(Drawable &pixmap, int source_x, int source_y);

	int rotate_90(int side);
	int get_w();
	int get_h();
	unsigned char* get_y_plane();
	unsigned char* get_u_plane();
	unsigned char* get_v_plane();
// Get the frame buffer itself
	int get_color_model();
	unsigned char** get_row_pointers();

// YUV status is stored in depth.
// Other color models are stored in bits_per_pixel after allocation.
	int w, h, depth;

private:
	int transfer_direct(unsigned char **rows, int in_w, int in_h);
	int transfer_scale(unsigned char **rows, int in_w, int in_h);
	int transfer_direct(VFrame *frame, int x1, int y1, int x2, int y2, int use_alpha);
	int transfer_scale(VFrame *frame, int x1, int y1, int x2, int y2, int use_alpha);

	int transfer_row_raw_direct(unsigned char *output, unsigned char *input, int width);
	int transfer_row_raw_scale(unsigned char *output, unsigned char *input, int *column_table);
	int transfer_row_direct_8(unsigned char *output, VPixel *input, int width, int use_alpha);
	int transfer_row_scale_8(unsigned char *output, VPixel *input, int *column_table, int use_alpha);
	int transfer_row_direct_16(unsigned TWO *output, VPixel *input, int width, int use_alpha);
	int transfer_row_scale_16(unsigned TWO *output, VPixel *input, int *column_table, int use_alpha);
	int transfer_row_direct_24(unsigned char *output, VPixel *input, int width, int use_alpha);
	int transfer_row_scale_24(unsigned char *output, VPixel *input, int *column_table, int use_alpha);
	int transfer_row_direct_32(unsigned char *output, VPixel *input, int width, int use_alpha);
	int transfer_row_scale_32(unsigned char *output, VPixel *input, int *column_table, int use_alpha);

// should be /= VMAX but >>= 8 is faster and good enough for screen output
#define FIX_RED_ALPHA \
	red *= alpha; \
	red >>= 8;

#define FIX_GREEN_ALPHA \
	green *= alpha; \
	green >>= 8;

#define FIX_BLUE_ALPHA \
	blue *= alpha; \
	blue >>= 8;

	inline void transfer_pixel_8(unsigned char *result, VPixel *input)
	{
#if (VMAX == 255)
		*result = (unsigned char)((input->r & 0xc0) + ((input->g & 0xe0) >> 2) + ((input->b & 0xe0) >> 5));
#else
		*result = (unsigned char)(((input->r & 0xc000) >> 8) + ((input->g & 0xe000) >> 10) + ((input->b & 0xe000) >> 13));
#endif
	};

	inline void transfer_raw_pixel_8(unsigned char *result, unsigned char *input)
	{
		*result = (unsigned char)((*input++ & 0xc0) + ((*input++ & 0xe0) >> 2) + ((*input & 0xe0) >> 5));
	};

	inline void transfer_pixel_8_alpha(unsigned char *result, VPixel *input)
	{
		unsigned int red, green, blue, alpha;
		alpha = input->a;
		red = input->r;
		green = input->g;
		blue = input->b;

#if (VMAX == 255)
		FIX_RED_ALPHA
		FIX_GREEN_ALPHA
		FIX_BLUE_ALPHA

		*result = (unsigned char)((red & 0xc0) + ((green & 0xe0) >> 2) + ((blue & 0xe0) >> 5));
#else
		alpha >>= 8;
		FIX_RED_ALPHA
		FIX_GREEN_ALPHA
		FIX_BLUE_ALPHA

		*result = (unsigned char)(((red & 0xc000) >> 8) + ((green & 0xe000) >> 10) + ((blue & 0xe000) >> 13));
#endif
	};

	inline void transfer_pixel_16(unsigned TWO *result, VPixel *input)
	{
#if (VMAX == 255)
		unsigned TWO red, green, blue;
		red = input->r;
		green = input->g;
		blue = input->b;

		*result = (unsigned TWO)(((red & 0xf8) << 8)
				 + ((green & 0xfc) << 3)
				 + ((blue & 0xf8) >> 3));
#else
		*result = (unsigned TWO)((input->r & 0xf800)
					+ ((input->g & 0xfc00) >> 5)
					+ ((input->b & 0xf800) >> 11));
#endif
	};

	inline void transfer_raw_pixel_16(unsigned TWO *result, unsigned char *input)
	{
		unsigned TWO red, green, blue;
		red = *input++;
		green = *input++;
		blue = *input;

		*result = (unsigned TWO)(((red & 0xf8) << 8)
				 + ((green & 0xfc) << 3)
				 + ((blue & 0xf8) >> 3));
	};

	inline void transfer_pixel_16_alpha(unsigned TWO *result, VPixel *input)
	{
		unsigned TWO red, green, blue, alpha;
		red = input->r;
		green = input->g;
		blue = input->b;
		alpha = input->a;

#if (VMAX == 255)
		FIX_RED_ALPHA
		FIX_GREEN_ALPHA
		FIX_BLUE_ALPHA

		*result = (unsigned TWO)(((red & 0xf8) << 8) + ((green & 0xfc) << 3) + ((blue & 0xf8) >> 3));
#else
		alpha >>= 8;

		FIX_RED_ALPHA
		FIX_GREEN_ALPHA
		FIX_BLUE_ALPHA

		*result = (unsigned TWO)((red & 0xf800) + ((green & 0xfc00) >> 5) + ((blue & 0xf800) >> 11));
#endif
	};
	
	inline void transfer_pixel_24(unsigned char **result, VPixel *input)
	{
#if (VMAX == 255)
		*(*result)++ = (unsigned char)input->b;
		*(*result)++ = (unsigned char)input->g;
		*(*result)++ = (unsigned char)input->r;
#else
		*(*result)++ = (unsigned char)(input->b >> 8);
		*(*result)++ = (unsigned char)(input->g >> 8);
		*(*result)++ = (unsigned char)(input->r >> 8);
#endif
	};

	inline void transfer_pixel_24_alpha(unsigned char **result, VPixel *input)
	{
		unsigned int red, green, blue, alpha;

// should be /= VMAX but this is faster and good enough for output of 24 bits
		alpha = input->a;
		red = input->r;
		green = input->g;
		blue = input->b;

#if (VMAX == 255)
		FIX_BLUE_ALPHA
		*(*result)++ = (unsigned char)blue;
		FIX_GREEN_ALPHA
		*(*result)++ = (unsigned char)green;
		FIX_RED_ALPHA
		*(*result)++ = (unsigned char)red;
#else
		alpha >>= 8;

		FIX_BLUE_ALPHA
		*(*result)++ = (unsigned char)(blue >> 8);
		FIX_GREEN_ALPHA
		*(*result)++ = (unsigned char)(green >> 8);
		FIX_RED_ALPHA
		*(*result)++ = (unsigned char)(red >> 8);
#endif
	};

	inline void transfer_raw_pixel_24(unsigned char *result, unsigned char *input)
	{
		result[2] = *input++;   // Red
		result[1] = *input++;   // green
		result[0] = *input;     // blue
	};

	int allocate_data();
	int delete_data();

	unsigned char *data;   // Points directly to the frame buffer
	unsigned char **row_data;   // Row pointers to the frame buffer
	XImage *ximage;
	XvImage *xv_image;
	XShmSegmentInfo shm_info;
	int xv_portid;
	int bits_per_pixel;
	int client_byte_order, server_byte_order;
	BC_Window *top_level;
	int last_pixmap_used;
	Drawable last_pixmap;
};

#endif
