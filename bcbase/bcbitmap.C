#include <string.h>
#include "bcbitmap.h"
#include "bcipc.h"
#include "bcresources.h"
#include "bctool.h"
#include "bcwindow.h"
#include "byteorder.h"
#include "colormodels.h"
#include "vframe.h"
#include <stdlib.h>




// Byte orders:
// 24 bpp packed:         bgr
// 24 bpp unpacked:       bgr0


BC_Bitmap::BC_Bitmap(BC_Window *top_level, int w, int h, int depth)
{
	this->top_level = top_level;
	this->w = w;
	this->h = h;
	this->depth = depth;

	ximage = 0;
	xv_image = 0;
	data = 0;
	client_byte_order = top_level->client_byte_order;
	server_byte_order = top_level->server_byte_order;
	last_pixmap_used = 0;

	allocate_data();
}


BC_Bitmap::~BC_Bitmap()
{
	delete_data();
}

int BC_Bitmap::allocate_data()
{
// Try shared memory
    if(top_level->get_resources()->use_shm)
	{
// Use YUV frame.  Must verify availability in BC_Window before calling this.
		if(depth == BC_YUV420P)
		{
			xv_portid = top_level->get_resources()->yuv_portid;
			xv_image = XvShmCreateImage(top_level->display, 
						xv_portid, 
						FOURCC_YV12,
						0, 
						w,
						h,
						&shm_info);
			shm_info.shmid = shmget(IPC_PRIVATE, 
				xv_image->data_size, 
				IPC_CREAT | 0777);
			if(shm_info.shmid < 0) perror("BC_Bitmap::allocate_data shmget");
			data = (unsigned char *)shmat(shm_info.shmid, NULL, 0);
			xv_image->data = shm_info.shmaddr = (char*)data;  // setting xv_image->data stops BadValue
			shm_info.readOnly = 0;
// Get the real parameters
			bits_per_pixel = 24;
			w = xv_image->width;
			h = xv_image->height;
		}
		else
// Use RGB frame
		{
	    	ximage = XShmCreateImage(top_level->display, 
				top_level->vis, 
				depth, 
				ZPixmap, 
				(char*)NULL, 
				&shm_info, 
				w, 
				h);
			shm_info.shmid = shmget(IPC_PRIVATE, 
				h * ximage->bytes_per_line + 4, 
				IPC_CREAT | 0777);
			if(shm_info.shmid < 0) perror("BC_Bitmap::allocate_data shmget");
			data = (unsigned char *)shmat(shm_info.shmid, NULL, 0);
			ximage->data = shm_info.shmaddr = (char*)data;  // setting ximage->data stops BadValue
			shm_info.readOnly = 0;
// This differs from the depth parameter of the top_level.
			bits_per_pixel = ximage->bits_per_pixel;
		}

// Crashes here if remote server.
		if(!XShmAttach(top_level->display, &shm_info))
		{
			perror("BC_Bitmap::allocate_data XShmAttach");
		}
		
		shmctl(shm_info.shmid, IPC_RMID, 0);
	}
	else
// Use unshared memory.
	{
// need to use bytes_per_line for some X servers
		data = 0;
// Use RGB frame
		ximage = XCreateImage(top_level->display, 
					top_level->vis, 
					depth, 
					ZPixmap, 
					0, 
					(char*)data, 
					w, 
					h, 
					8, 
					0);

		data = (unsigned char*)malloc(h * ximage->bytes_per_line + 4);

		XDestroyImage(ximage);

		ximage = XCreateImage(top_level->display, 
					top_level->vis, 
					depth, 
					ZPixmap, 
					0, 
					(char*)data, 
					w, 
					h, 
					8, 
					0);
// This differs from the depth parameter of the top_level.
		bits_per_pixel = ximage->bits_per_pixel;
	}

	if(depth != BC_YUV420P)
	{
		row_data = new unsigned char*[h];
		for(int i = 0; i < h; i++)
		{
			row_data[i] = &data[i * ximage->bytes_per_line];
		}
	}
	return 0;
}

int BC_Bitmap::delete_data()
{
	if(data)
	{
		if(depth == BC_YUV420P)
		{
			if(last_pixmap_used) XvStopVideo(top_level->display, xv_portid, last_pixmap);
			XShmDetach(top_level->display, &shm_info);
			shmdt(shm_info.shmaddr);
			shmctl(shm_info.shmid, IPC_RMID, 0);
			bc_remove_shmem_id(shm_info.shmid);
			XFree(xv_image);
		}
		else
		if(top_level->get_resources()->use_shm)
		{
			XShmDetach(top_level->display, &shm_info);
			XDestroyImage(ximage);
			shmdt(shm_info.shmaddr);
			shmctl(shm_info.shmid, IPC_RMID, 0);
			bc_remove_shmem_id(shm_info.shmid);
			delete row_data;
		}
		else
		{
			XDestroyImage(ximage);
			delete row_data;
		}

// data is automatically freed by XDestroyImage
		data = 0;
	}
return 0;
}

int BC_Bitmap::write_drawable(Drawable &pixmap, 
							int dest_x, 
							int dest_y, 
							int source_x, 
							int source_y, 
							int w, 
							int h,
							int dont_wait)
{
    if(top_level->get_resources()->use_shm)
	{
// The shared memory isn't writable until this is called.
//		if(dont_wait) XSync(top_level->display, False);
		if(depth == BC_YUV420P)
		{
			XvShmPutImage(top_level->display, xv_portid, pixmap, top_level->gc,
						xv_image, source_x, source_y, this->w - source_x, 
						this->h - source_y, dest_x, dest_y, w, h, False);
			last_pixmap = pixmap;
			last_pixmap_used = 1;
		}
		else
		{
        	XShmPutImage(top_level->display, pixmap, top_level->gc, ximage,
                    	 source_x, source_y, dest_x, dest_y, w, h, False);
		}
// Force the X server into not only flushing but processing all requests.
// This allows the shared memory to be written to again.
		if(!dont_wait) XSync(top_level->display, 0);
	}
    else
        XPutImage(top_level->display, pixmap, top_level->gc, ximage,
                  source_x, source_y, dest_x, dest_y, w, h);
return 0;
}


// the bitmap must be wholly contained in the source during a GetImage
int BC_Bitmap::read_drawable(Drawable &pixmap, int source_x, int source_y)
{
	if(top_level->get_resources()->use_shm)
		XShmGetImage(top_level->display, pixmap, ximage, source_x, source_y, 0xffffffff);
	else
		XGetSubImage(top_level->display, pixmap, source_x, source_y, w, h, 0xffffffff, ZPixmap, ximage, 0, 0);
}

// ============================ Communication with VFrames

int BC_Bitmap::read_frame(VFrame *frame, 
		int x1, int y1, int x2, int y2, int use_alpha)
{
	if(frame->get_color_model() == VFRAME_RGB888) 
	{
		return read_frame(frame->get_rows(), frame->get_w(), frame->get_h());
	}

	float zoom_x, zoom_y;

	zoom_x = (float)w / (x2 - x1);
	zoom_y = (float)h / (y2 - y1);

	if(frame->get_color_model() == VFRAME_VPIXEL)
	{
		if(zoom_x == 1 && zoom_y == 1)
			transfer_direct(frame, x1, y1, x2, y2, use_alpha);
		else
			transfer_scale(frame, x1, y1, x2, y2, use_alpha);
	}
return 0;
}

int BC_Bitmap::read_frame(unsigned char **rows, int in_w, int in_h)
{
	float zoom_x, zoom_y;

	zoom_x = (float)w / in_w;
	zoom_y = (float)h / in_h;

	if(zoom_x == 1 && zoom_y == 1)
		transfer_direct(rows, in_w, in_h);
	else
		transfer_scale(rows, in_w, in_h);
}

int BC_Bitmap::get_color_model()
{
	switch(bits_per_pixel)
	{
		case 8:  return BC_RGB8;     break;
		case 16: return BC_RGB565;   break;
		case 24: return BC_BGR888;   break;
		case 32: return BC_BGR8888; break;
	}

	switch(depth)
	{
		case BC_YUV420P:
			return BC_YUV420P;
			break;
	}
	return -1;
}

unsigned char** BC_Bitmap::get_row_pointers()
{
	return row_data;
}

unsigned char* BC_Bitmap::get_y_plane()
{
	return data + xv_image->offsets[0];
}

unsigned char* BC_Bitmap::get_v_plane()
{
	return data + xv_image->offsets[1];
}

unsigned char* BC_Bitmap::get_u_plane()
{
	return data + xv_image->offsets[2];
}


int BC_Bitmap::get_w() { return w; }

int BC_Bitmap::get_h() { return h; }





// ================================ frame operations

int BC_Bitmap::transfer_direct(unsigned char **rows, int in_w, int in_h)
{
	int y_in, y_out;

	for(y_in = 0; y_in < in_h; y_in++)
	{
		transfer_row_raw_direct(row_data[y_in], rows[y_in], in_w);
		if(client_byte_order != server_byte_order)
			swap_bytes(bits_per_pixel / 8, row_data[y_in], w * bits_per_pixel / 8);
	}
	return 0;
}


#define TRANSFER_DIRECT_LOOP1 \
	for(y_in = y1, y_out = 0; y_in < y2; y_in++, y_out++) \
	{

#define TRANSFER_DIRECT_LOOP2 \
		if(client_byte_order != server_byte_order) \
			swap_bytes(bits_per_pixel / 8, row_data[y_out], width * bits_per_pixel / 8); \
	}

int BC_Bitmap::transfer_direct(VFrame *frame, int x1, int y1, int x2, int y2, int use_alpha)
{
	int y_in, y_out, width;
	width = x2 - x1;

	switch(bits_per_pixel)
	{
		case 8:
			TRANSFER_DIRECT_LOOP1
				transfer_row_direct_8(row_data[y_out], &(((VPixel**)frame->get_rows())[y_in][x1]), width, use_alpha);
			TRANSFER_DIRECT_LOOP2
			break;
		case 16:
			TRANSFER_DIRECT_LOOP1
				transfer_row_direct_16((unsigned TWO*)row_data[y_out], &(((VPixel**)frame->get_rows())[y_in][x1]), width, use_alpha);
			TRANSFER_DIRECT_LOOP2
			break;
		case 24:
			TRANSFER_DIRECT_LOOP1
				transfer_row_direct_24(row_data[y_out], &(((VPixel**)frame->get_rows())[y_in][x1]), width, use_alpha);
			TRANSFER_DIRECT_LOOP2
			break;
		case 32:
			TRANSFER_DIRECT_LOOP1
				transfer_row_direct_32(row_data[y_out], &(((VPixel**)frame->get_rows())[y_in][x1]), width, use_alpha);
			TRANSFER_DIRECT_LOOP2
			break;
	}
	return 0;
}

int BC_Bitmap::transfer_scale(unsigned char **rows, int in_w, int in_h)
{
	int *column_table;    // table of input columns for each bitmap column
	int *row_table;       // table of input rows for each bitmap row
	float hscale = (float)in_w / w;
	float vscale = (float)in_h / h;
	int i;
	
	column_table = new int[w];
	row_table = new int[h];

	for(i = 0; i < w; i++)
	{
		column_table[i] = (int)(hscale * i) * 3;
	}

	for(i = 0; i < h; i++)
	{
		row_table[i] = (int)(vscale * i);
	}

	int y_in, y_out;
	for(y_out = 0; y_out < h; y_out++)
	{
		transfer_row_raw_scale(row_data[y_out], 
			rows[row_table[y_out]], column_table);
		if(client_byte_order != server_byte_order)
			swap_bytes(bits_per_pixel / 8, row_data[y_out], w * bits_per_pixel / 8);
	}
	
	delete [] column_table;
	delete [] row_table;
	
	return 0;
}

#define TRANSFER_REDUCE_LOOP1 \
	for(y_out = 0; y_out < h; y_out++) \
	{

#define TRANSFER_REDUCE_LOOP2 \
		if(client_byte_order != server_byte_order) \
			swap_bytes(bits_per_pixel / 8, row_data[y_out], w * bits_per_pixel / 8); \
	}

int BC_Bitmap::transfer_scale(VFrame *frame, int x1, int y1, int x2, int y2, int use_alpha)
{
	int y_out, i;
	float width = x2 - x1;
	float height = y2 - y1;
	int *column_table;    // table of input columns for each bitmap column
	int *row_table;       // table of input rows for each bitmap row
	float hscale = width / w;
	float vscale = height / h;

	column_table = new int[w];
	row_table = new int[h];

	for(i = 0; i < w; i++)
	{
		column_table[i] = (int)(hscale * i);
	}

	for(i = 0; i < h; i++)
	{
		row_table[i] = (int)(vscale * i) + y1;
	}

	switch(bits_per_pixel)
	{
		case 8:
			TRANSFER_REDUCE_LOOP1
				transfer_row_scale_8(row_data[y_out], &(((VPixel**)frame->get_rows())[row_table[y_out]][x1]), column_table, use_alpha);
			}
			break;
		case 16:
			TRANSFER_REDUCE_LOOP1
				transfer_row_scale_16((unsigned TWO*)row_data[y_out], &(((VPixel**)frame->get_rows())[row_table[y_out]][x1]), column_table, use_alpha);
			TRANSFER_REDUCE_LOOP2
			break;
		case 24:
			TRANSFER_REDUCE_LOOP1
				transfer_row_scale_24(row_data[y_out], &(((VPixel**)frame->get_rows())[row_table[y_out]][x1]), column_table, use_alpha);
			TRANSFER_REDUCE_LOOP2
			break;
		case 32:
			TRANSFER_REDUCE_LOOP1
				transfer_row_scale_32(row_data[y_out], &(((VPixel**)frame->get_rows())[row_table[y_out]][x1]), column_table, use_alpha);
			TRANSFER_REDUCE_LOOP2
			break;
	}
	delete [] column_table;
	delete [] row_table;
	return 0;
}













// ==================================== row operations

int BC_Bitmap::transfer_row_raw_direct(unsigned char *output, unsigned char *input, int width)
{
	unsigned char *input_end = input + width * 3;
	unsigned char *input_ptr = input, *output_ptr = output;
	switch(bits_per_pixel)
	{
		case 8:
			while(input < input_end)
			{
				transfer_raw_pixel_8(output++, input);
				input += 3;
			}
			break;
		
		case 16:
			while(input < input_end)
			{
				transfer_raw_pixel_16((unsigned TWO*)output, input);
				output += 2;
				input += 3;
			}
			break;
		
		case 24:
			while(input < input_end)
			{
				transfer_raw_pixel_24(output, input);
				output += 3;
				input += 3;
			}
			break;
		
		case 32:
			while(input < input_end)
			{
				transfer_raw_pixel_24(output, input);
				output += 4;
				input += 3;
			}
			break;
	}
return 0;
}


int BC_Bitmap::transfer_row_raw_scale(unsigned char *output, unsigned char *input, int *column_table)
{
	unsigned char *input_ptr = input, *output_ptr = output;
	switch(bits_per_pixel)
	{
		case 8:
			for( int x = 0; x < w; x++)
			{
				transfer_raw_pixel_8(output++, &input[column_table[x]]);
			}
			break;

		case 16:
			for( int x = 0; x < w; x++)
			{
				transfer_raw_pixel_16((unsigned TWO*)output, &input[column_table[x]]);
				output += 2;
			}
			break;

		case 24:
			for( int x = 0; x < w; x++)
			{
				transfer_raw_pixel_24(output, &input[column_table[x]]);
				output += 3;
//				input += 3;
			}
			break;

		case 32:
			for( int x = 0; x < w; x++)
			{
				transfer_raw_pixel_24(output, &input[column_table[x]]);
				output += 4;
//				input += 3;
			}
			break;
	}
return 0;
}

int BC_Bitmap::transfer_row_direct_8(unsigned char *output, VPixel *input, int width, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_8_alpha(output++, &input[x]);
		}
	else
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_8(output++, &input[x]);
		}
return 0;
}

int BC_Bitmap::transfer_row_scale_8(unsigned char *output, VPixel *input, int *column_table, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_8_alpha(output++, &input[column_table[x]]);
		}
	else
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_8(output++, &input[column_table[x]]);
		}
return 0;
}

int BC_Bitmap::transfer_row_direct_16(unsigned TWO *output, VPixel *input, int width, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_16_alpha(output++, &input[x]);
		}
	else
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_16(output++, &input[x]);
		}
return 0;
}

int BC_Bitmap::transfer_row_scale_16(unsigned TWO *output, VPixel *input, int *column_table, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_16_alpha(output++, &input[column_table[x]]);
		}
	else
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_16(output++, &input[column_table[x]]);
		}
return 0;
}

int BC_Bitmap::transfer_row_direct_24(unsigned char *output, VPixel *input, int width, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_24_alpha(&output, &input[x]);
		}
	else
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_24(&output, &input[x]);
		}
return 0;
}

int BC_Bitmap::transfer_row_scale_24(unsigned char *output, VPixel *input, int *column_table, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_24_alpha(&output, &input[column_table[x]]);
		}
	else
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_24(&output, &input[column_table[x]]);
		}
return 0;
}

int BC_Bitmap::transfer_row_direct_32(unsigned char *output, VPixel *input, int width, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_24_alpha(&output, &input[x]);
			output++;
		}
	else
		for( int x = 0; x < width; x++)
		{
			transfer_pixel_24(&output, &input[x]);
			output++;
		}
return 0;
}

int BC_Bitmap::transfer_row_scale_32(unsigned char *output, VPixel *input, int *column_table, int use_alpha)
{
	if(use_alpha)
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_24_alpha(&output, &input[column_table[x]]);
			output++;
		}
	else
		for( int x = 0; x < w; x++)
		{
			transfer_pixel_24(&output, &input[column_table[x]]);
			output++;
		}
return 0;
}












// ============================ pixel operations



int BC_Bitmap::rotate_90(int side)
{
	unsigned char** new_data;
	int i, j, k, l, m, step = bits_per_pixel / 8;
	int row_bytes = side * step;

	new_data = new unsigned char*[side];
	for(i = 0; i < side; i++) new_data[i] = new unsigned char[(side + 1) * step];

	for(i = 0; i < side; i++)
	{
// scan a row from row_data to a column from new_data
		j = (side - i) * step;

		for(k = 0, m = 0; k < side; k++, m += step)
		{
			for(l = 0; l < step; l++)
			{
				new_data[k][j + l] = row_data[i][m + l];
			}
		}
	}

	for(i = 0; i < side; i++)
	{
		for(j = 0; j < row_bytes; j++)
		{
			row_data[i][j] = new_data[i][j];
		}
	}
	
	for(i = 0; i < side; i++) delete new_data[i];
	delete new_data;
return 0;
}
