#include "bcbitmap.h"
#include "bcipc.h"
#include "bcresources.h"
#include "bcwindow.h"
#include "colormodels.h"
#include "vframe.h"

#include <string.h>
#include <X11/extensions/Xvlib.h>


BC_Bitmap::BC_Bitmap(BC_WindowBase *parent_window, unsigned char *png_data)
{
// Decompress data into a temporary vframe
	VFrame frame;

	frame.read_png(png_data);

// Initialize the bitmap
	initialize(parent_window, 
		frame.get_w(), 
		frame.get_h(), 
		parent_window->get_color_model(), 
		0);

// Copy the vframe to the bitmap
	read_frame(&frame, 0, 0, w, h, 0);
}

BC_Bitmap::BC_Bitmap(BC_WindowBase *parent_window, VFrame *frame)
{
// Initialize the bitmap
	initialize(parent_window, 
		frame->get_w(), 
		frame->get_h(), 
		parent_window->get_color_model(), 
		0);

// Copy the vframe to the bitmap
	read_frame(frame, 0, 0, w, h, 1);
}

BC_Bitmap::BC_Bitmap(BC_WindowBase *parent_window, int w, int h, int color_model, int use_shm)
{
	initialize(parent_window, 
		w, 
		h, 
		color_model, 
		use_shm ? parent_window->get_resources()->use_shm : 0);
}

BC_Bitmap::~BC_Bitmap()
{
	delete_data();
}

int BC_Bitmap::initialize(BC_WindowBase *parent_window, int w, int h, int color_model, int use_shm)
{
	this->parent_window = parent_window;
	this->top_level = parent_window->top_level;
	this->w = w;
	this->h = h;
	this->color_model = color_model;
	this->use_shm = use_shm;
	this->bg_color = parent_window->bg_color;
	ximage[0] = 0;
	xv_image[0] = 0;
	data[0] = 0;
	last_pixmap_used = 0;
	last_pixmap = 0;
	current_ringbuffer = 0;
	ring_buffers = 1;

	allocate_data();
	return 0;
}

int BC_Bitmap::match_params(int w, int h, int color_model, int use_shm)
{
	if(this->w != w ||
		this->h != h ||
		this->color_model != color_model ||
		this->use_shm != use_shm)
	{
		delete_data();
		initialize(parent_window, w, h, color_model, use_shm);
	}

	return 0;
}

int BC_Bitmap::params_match(int w, int h, int color_model, int use_shm)
{
	int result = 0;
	if(this->w == w &&
		this->h == h &&
		this->color_model == color_model)
	{
		if(use_shm == this->use_shm || use_shm == INFINITY)
			result = 1;
	}

	return result;
}


int BC_Bitmap::allocate_data()
{
	int want_row_pointers = 1;

// Shared memory available
    if(use_shm)
	{
		switch(color_model)
		{
// Planar YUV.  Must use BC_WindowBase::accel_available before calling this.
			case BC_YUV420P:
			case BC_YUV422P:
// Packed YUV
			case BC_YUV422:
//printf("BC_Bitmap::allocate_data 1\n");
				ring_buffers = BITMAP_RING;
				xv_portid = top_level->xvideo_port_id;
// Create the X Image
				xv_image[0] = XvShmCreateImage(top_level->display, 
							xv_portid, 
							cmodel_bc_to_x(color_model),
							0, 
							w,
							h,
							&shm_info);
// Create the shared memory
				shm_info.shmid = shmget(IPC_PRIVATE, 
					xv_image[0]->data_size * ring_buffers + 4, 
					IPC_CREAT | 0777);
//printf("BC_Bitmap %d\n", xv_image[0]->data_size * ring_buffers + 4);
				if(shm_info.shmid < 0) perror("BC_Bitmap::allocate_data shmget");
				data[0] = (unsigned char *)shmat(shm_info.shmid, NULL, 0);
// setting ximage->data stops BadValue
				xv_image[0]->data = shm_info.shmaddr = (char*)data[0];
				shm_info.readOnly = 0;

// Get the real parameters
				w = xv_image[0]->width;
				h = xv_image[0]->height;
//printf("BC_Bitmap::allocate_data %d %d\n", w, h);

// Create remaining X Images
				for(int i = 1; i < ring_buffers; i++)
				{
					data[i] = data[0] + xv_image[0]->data_size * i;
					xv_image[i] = XvShmCreateImage(top_level->display, 
								xv_portid, 
								cmodel_bc_to_x(color_model),
								(char*)data[i], 
								w,
								h,
								&shm_info);
					xv_image[i]->data = (char*)data[i];
				}
				
				if(color_model == BC_YUV422)
				{
					bytes_per_line = w * 2;
					bits_per_pixel = 2;
					want_row_pointers = 1;
				}
				else
				{
					bytes_per_line = 0;
					bits_per_pixel = 0;
					want_row_pointers = 0;
				}
//printf("BC_Bitmap %d %d %d %d\n", color_model, xv_image[0]->data_size, bits_per_pixel, bytes_per_line);
				break;

			default:
// RGB
				ring_buffers = BITMAP_RING;
// Create first X Image
		    	ximage[0] = XShmCreateImage(top_level->display, 
					top_level->vis, 
					get_default_depth(), 
					ZPixmap, 
					(char*)NULL, 
					&shm_info, 
					w, 
					h);

// Create shared memory
				shm_info.shmid = shmget(IPC_PRIVATE, 
					h * ximage[0]->bytes_per_line * ring_buffers + 4, 
					IPC_CREAT | 0777);
				if(shm_info.shmid < 0) 
					perror("BC_Bitmap::allocate_data shmget");
				data[0] = (unsigned char *)shmat(shm_info.shmid, NULL, 0);
				ximage[0]->data = shm_info.shmaddr = (char*)data[0];  // setting ximage->data stops BadValue
				shm_info.readOnly = 0;

// Get the real parameters
				bits_per_pixel = ximage[0]->bits_per_pixel;
				bytes_per_line = ximage[0]->bytes_per_line;

// Create remaining X Images
				for(int i = 1; i < ring_buffers; i++)
				{
					data[i] = data[0] + h * ximage[0]->bytes_per_line * i;
					ximage[i] = XShmCreateImage(top_level->display, 
											top_level->vis, 
											get_default_depth(), 
											ZPixmap, 
											(char*)data[i], 
											&shm_info, 
											w, 
											h);
					ximage[i]->data = (char*)data[i];
				}
				break;
		}

		if(!XShmAttach(top_level->display, &shm_info))
		{
			perror("BC_Bitmap::allocate_data XShmAttach");
		}

		shmctl(shm_info.shmid, IPC_RMID, 0);
	}
	else
// Use unshared memory.
	{
		ring_buffers = 1;
// need to use bytes_per_line for some X servers
		data[0] = 0;
// Use RGB frame
		ximage[0] = XCreateImage(top_level->display, 
					top_level->vis, 
					get_default_depth(), 
					get_default_depth() == 1 ? XYBitmap : ZPixmap, 
					0, 
					(char*)data[0], 
					w, 
					h, 
					8, 
					0);

		data[0] = (unsigned char*)malloc(h * ximage[0]->bytes_per_line + 4);

		XDestroyImage(ximage[0]);

		ximage[0] = XCreateImage(top_level->display, 
					top_level->vis, 
					get_default_depth(), 
					get_default_depth() == 1 ? XYBitmap : ZPixmap, 
					0, 
					(char*)data[0], 
					w, 
					h, 
					8, 
					0);
// This differs from the depth parameter of the top_level.
		bits_per_pixel = ximage[0]->bits_per_pixel;
		bytes_per_line = ximage[0]->bytes_per_line;
	}

// Create row pointers
	if(want_row_pointers)
	{
		for(int j = 0; j < ring_buffers; j++)
		{
			row_data[j] = new unsigned char*[h];
			for(int i = 0; i < h; i++)
			{
				row_data[j][i] = &data[j][i * bytes_per_line];
			}
		}
	}
	return 0;
}

int BC_Bitmap::delete_data()
{
	if(data[0])
	{
		if(use_shm)
		{
			switch(color_model)
			{
				case BC_YUV420P:
				case BC_YUV422P:
				case BC_YUV422:
//printf("BC_Bitmap::delete_data 1\n");
					if(last_pixmap_used) XvStopVideo(top_level->display, xv_portid, last_pixmap);
					for(int i = 0; i < ring_buffers; i++)
					{
						XFree(xv_image[i]);
					}
					XShmDetach(top_level->display, &shm_info);
					XvUngrabPort(top_level->display, xv_portid, CurrentTime);

 					shmdt(shm_info.shmaddr);
 					shmctl(shm_info.shmid, IPC_RMID, 0);
					break;

				default:
					for(int i = 0; i < ring_buffers; i++)
					{
						XDestroyImage(ximage[i]);
						delete row_data[i];
					}
					XShmDetach(top_level->display, &shm_info);

					shmdt(shm_info.shmaddr);
					shmctl(shm_info.shmid, IPC_RMID, 0);
					break;
			}
		}
		else
		{
			XDestroyImage(ximage[0]);
			delete row_data[0];
		}

// data is automatically freed by XDestroyImage
		data[0] = 0;
		last_pixmap_used = 0;
	}
	return 0;
}

int BC_Bitmap::get_default_depth()
{
	if(color_model == BC_TRANSPARENCY)
		return 1;
	else
		return top_level->default_depth;
}


int BC_Bitmap::set_bg_color(int color)
{
	this->bg_color = color;
	return 0;
}

int BC_Bitmap::invert()
{
	for(int j = 0; j < ring_buffers; j++)
		for(int k = 0; k < h; k++)
			for(int i = 0; i < bytes_per_line; i++)
			{
				row_data[j][k][i] ^= 0xff;
			}
	return 0;
}

int BC_Bitmap::write_drawable(Drawable &pixmap, 
							GC &gc,
							int dest_x, 
							int dest_y, 
							int source_x, 
							int source_y, 
							int dest_w, 
							int dest_h,
							int dont_wait)
{
	return write_drawable(pixmap, 
		gc,
		source_x, 
		source_y, 
		get_w() - source_x,
		get_h() - source_y,
		dest_x, 
		dest_y, 
		dest_w, 
		dest_h, 
		dont_wait);
}



int BC_Bitmap::write_drawable(Drawable &pixmap, 
		GC &gc,
		int source_x, 
		int source_y, 
		int source_w,
		int source_h,
		int dest_x, 
		int dest_y, 
		int dest_w, 
		int dest_h, 
		int dont_wait)
{
//printf("BC_Bitmap::write_drawable 1 %p\n", this);fflush(stdout);
    if(use_shm)
	{
//		if(dont_wait) XSync(top_level->display, False);

		if(hardware_scaling())
		{
// printf("BC_Bitmap::write_drawable %d %d %d %d -> %d %d %d %d\n", source_x, 
// 				source_y, 
// 				source_w, 
// 				source_h, 
// 				dest_x, 
// 				dest_y, 
// 				dest_w, 
// 				dest_h);
			XvShmPutImage(top_level->display, 
				xv_portid, 
				pixmap, 
				gc,
				xv_image[current_ringbuffer], 
				source_x, 
				source_y, 
				source_w, 
				source_h, 
				dest_x, 
				dest_y, 
				dest_w, 
				dest_h, 
				False);
// Need to pass these to the XvStopVideo
			last_pixmap = pixmap;
			last_pixmap_used = 1;
		}
		else
		{
        	XShmPutImage(top_level->display, 
				pixmap, 
				gc, 
				ximage[current_ringbuffer], 
				source_x, 
				source_y, 
				dest_x, 
				dest_y, 
				dest_w, 
				dest_h, 
				False);
		}

// Force the X server into processing all requests.
// This allows the shared memory to be written to again.
		if(!dont_wait) XSync(top_level->display, 0);
	}
    else
	{
        XPutImage(top_level->display, 
			pixmap, 
			gc, 
			ximage[current_ringbuffer], 
			source_x, 
			source_y, 
			dest_x, 
			dest_y, 
			dest_w, 
			dest_h);
	}

//printf("BC_Bitmap %d\n", current_ringbuffer);
	current_ringbuffer++;
	if(current_ringbuffer >= ring_buffers) current_ringbuffer = 0;
//printf("BC_Bitmap::write_drawable 2\n");fflush(stdout);
	return 0;
}




// the bitmap must be wholly contained in the source during a GetImage
int BC_Bitmap::read_drawable(Drawable &pixmap, int source_x, int source_y)
{
	if(use_shm)
		XShmGetImage(top_level->display, pixmap, ximage[current_ringbuffer], source_x, source_y, 0xffffffff);
	else
		XGetSubImage(top_level->display, pixmap, source_x, source_y, w, h, 0xffffffff, ZPixmap, ximage[current_ringbuffer], 0, 0);
	return 0;
}

// ============================ Decoding VFrames

int BC_Bitmap::read_frame(VFrame *frame, 
	int x1, 
	int y1, 
	int x2, 
	int y2, 
	int use_alpha)
{
	return read_frame(frame, use_alpha,
		0, 0, frame->get_w(), frame->get_h(),
		x1, y1, x2 - x1, y2 - y1);
}


int BC_Bitmap::read_frame(VFrame *frame, int use_alpha,
		int in_x, int in_y, int in_w, int in_h,
		int out_x, int out_y, int out_w, int out_h)
{
	switch(color_model)
	{
// Hardware acceleration
		case BC_YUV420P:
			if(frame->get_color_model() == color_model)
			{
				memcpy(get_y_plane(), frame->get_y(), w * h);
				memcpy(get_u_plane(), frame->get_u(), w * h / 4);
				memcpy(get_v_plane(), frame->get_v(), w * h / 4);
				break;
			}

		case BC_YUV422P:
			if(frame->get_color_model() == color_model)
			{
				memcpy(get_y_plane(), frame->get_y(), w * h);
				memcpy(get_u_plane(), frame->get_u(), w * h / 2);
				memcpy(get_v_plane(), frame->get_v(), w * h / 2);
				break;
			}

		case BC_YUV422:
			if(frame->get_color_model() == color_model)
			{
				memcpy(get_data(), frame->get_data(), w * h + w * h);
				break;
			}

// Software only
		default:
			cmodel_transfer(row_data[current_ringbuffer], 
				frame->get_rows(),
				get_y_plane(),
				get_u_plane(),
				get_v_plane(),
				frame->get_y(),
				frame->get_u(),
				frame->get_v(),
				in_x, 
				in_y, 
				in_w, 
				in_h,
				out_x, 
				out_y, 
				out_w, 
				out_h,
				frame->get_color_model(), 
				color_model,
				bg_color,
				frame->get_w(),
				w);
			break;
	}


	return 0;
}

long BC_Bitmap::get_shm_id()
{
	return shm_info.shmid;
}

long BC_Bitmap::get_shm_size()
{
	if(xv_image[0])
		return xv_image[0]->data_size * ring_buffers;
	else
		return h * ximage[0]->bytes_per_line;
}

long BC_Bitmap::get_shm_offset()
{
	if(xv_image[0])
		return xv_image[0]->data_size * current_ringbuffer;
	else
	if(ximage[0])
		return h * ximage[0]->bytes_per_line * current_ringbuffer;
	else
		return 0;
}

long BC_Bitmap::get_y_shm_offset()
{
	if(xv_image[0])
		return get_shm_offset() + xv_image[current_ringbuffer]->offsets[0];
	else
		return 0;
}

long BC_Bitmap::get_u_shm_offset()
{
	if(xv_image[0])
		return get_shm_offset() + xv_image[current_ringbuffer]->offsets[2];
	else
		return 0;
}

long BC_Bitmap::get_v_shm_offset()
{
	if(xv_image[0])
		return get_shm_offset() + xv_image[current_ringbuffer]->offsets[1];
	else
		return 0;
}

long BC_Bitmap::get_y_offset()
{
	if(xv_image[0])
		return xv_image[current_ringbuffer]->offsets[0];
	else
		return 0;
}

long BC_Bitmap::get_u_offset()
{
	if(xv_image[0])
		return xv_image[current_ringbuffer]->offsets[2];
	else
		return 0;
}

long BC_Bitmap::get_v_offset()
{
	if(xv_image[0])
		return xv_image[current_ringbuffer]->offsets[1];
	else
		return 0;
}


unsigned char** BC_Bitmap::get_row_pointers()
{
	return row_data[current_ringbuffer];
}

unsigned char* BC_Bitmap::get_data()
{
//printf("BC_Bitmap::get_data %d %p\n",current_ringbuffer , data[current_ringbuffer]);
	return data[current_ringbuffer];
}

unsigned char* BC_Bitmap::get_y_plane()
{
	if(color_model == BC_YUV420P ||
		color_model == BC_YUV422P)
		return data[current_ringbuffer] + xv_image[current_ringbuffer]->offsets[0];
	else
		return 0;
}

unsigned char* BC_Bitmap::get_v_plane()
{
	if(color_model == BC_YUV420P ||
		color_model == BC_YUV422P)
		return data[current_ringbuffer] + xv_image[current_ringbuffer]->offsets[1];
	else
		return 0;
}

unsigned char* BC_Bitmap::get_u_plane()
{
	if(color_model == BC_YUV420P ||
		color_model == BC_YUV422P)
		return data[current_ringbuffer] + xv_image[current_ringbuffer]->offsets[2];
	else
		return 0;
}

void BC_Bitmap::rewind_ringbuffer()
{
	current_ringbuffer--;
	if(current_ringbuffer < 0) current_ringbuffer = ring_buffers - 1;
}

int BC_Bitmap::hardware_scaling()
{
	return (get_color_model() == BC_YUV420P || 
		get_color_model() == BC_YUV422P || 
		get_color_model() == BC_YUV422);
}

int BC_Bitmap::get_w() { return w; }

int BC_Bitmap::get_h() { return h; }

int BC_Bitmap::get_color_model() { return color_model; }



