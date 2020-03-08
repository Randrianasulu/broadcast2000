#include <string.h>
#include "bccapture.h"
#include "bcresources.h"
#include "byteorder.h"
#include "vframe.h"
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xutil.h>




// Byte orders:
// 24 bpp packed:         bgr
// 24 bpp unpacked:       0bgr


BC_Capture::BC_Capture(int w, int h, char *display_text)
{
	this->w = w;
	this->h = h;

	data = 0;
	use_shm = 1;
	import_pixel = 0;
	init_window(display_text);
	allocate_data();
	import_pixel = new BC_ImportPixel(bits_per_pixel, server_byte_order);
}


BC_Capture::~BC_Capture()
{
	delete import_pixel;
	delete_data();
}

int BC_Capture::init_window(char *display_text)
{
	if(display_text && display_text[0] == 0) display_text = NULL;
	if((display = XOpenDisplay(display_text)) == NULL)
	{
  		printf("cannot connect to X server.\n");
  		if(getenv("DISPLAY") == NULL)
    		printf("'DISPLAY' environment variable not set.\n");
  		exit(-1);
		return 1;
 	}

	screen = DefaultScreen(display);
	rootwin = RootWindow(display, screen);
	vis = DefaultVisual(display, screen);
	depth = DefaultDepth(display, screen);
	client_byte_order = get_byte_order();
	server_byte_order = XImageByteOrder(display);
	if(server_byte_order == MSBFirst)
		server_byte_order = 0;
	else
		server_byte_order = 1;

// test shared memory
// This doesn't ensure the X Server is on the local host
    if(use_shm && !XShmQueryExtension(display))
    {
        use_shm = 0;
    }
return 0;
}


int BC_Capture::get_w() { return w; return 0;
}
int BC_Capture::get_h() { return h; return 0;
}

// Capture a frame from the screen
int BC_Capture::capture_frame(VFrame *frame, int x1, int y1)
{
	int i, j;
	unsigned char *input_row;
	BC_CapturePixel *output_row;

	if(x1 < 0) x1 = 0;
	if(y1 < 0) y1 = 0;
	if(x1 > get_top_w() - w) x1 = get_top_w() - w;
	if(y1 > get_top_h() - h) y1 = get_top_h() - h;


// Read the raw data
	if(use_shm)
		XShmGetImage(display, rootwin, ximage, x1, y1, 0xffffffff);
	else
		XGetSubImage(display, rootwin, x1, y1, w, h, 0xffffffff, ZPixmap, ximage, 0, 0);

// Convert to VFrame
	for(i = 0; i < h; i++)
	{
		input_row = row_data[i];
		output_row = (BC_CapturePixel*)((unsigned char*)frame->get_rows()[i]);
		for(j = 0; j < w; j++)
		{
			import_pixel->import_pixel(*output_row++, &input_row);
		}
	}

	return 0;
return 0;
}

int BC_Capture::allocate_data()
{
// try shared memory
    if(use_shm)
	{
	    ximage = XShmCreateImage(display, vis, depth, ZPixmap, (char*)NULL, &shm_info, w, h);

		shm_info.shmid = shmget(IPC_PRIVATE, h * ximage->bytes_per_line, IPC_CREAT | 0777);
		if(shm_info.shmid < 0) perror("BC_Capture::allocate_data shmget");
		data = (unsigned char *)shmat(shm_info.shmid, NULL, 0);
		shmctl(shm_info.shmid, IPC_RMID, 0);
		ximage->data = shm_info.shmaddr = (char*)data;  // setting ximage->data stops BadValue
		shm_info.readOnly = 0;

// Crashes here if remote server.
		BC_Resources::error = 0;
		XShmAttach(display, &shm_info);
    	XSync(display, False);
		if(BC_Resources::error)
		{
			XDestroyImage(ximage);
			shmdt(shm_info.shmaddr);
			use_shm = 0;
		}
	}
	
	if(!use_shm)
	{
// need to use bytes_per_line for some X servers
		data = 0;
		ximage = XCreateImage(display, vis, depth, ZPixmap, 0, (char*)data, w, h, 8, 0);
		data = (unsigned char*)malloc(h * ximage->bytes_per_line);
		XDestroyImage(ximage);

		ximage = XCreateImage(display, vis, depth, ZPixmap, 0, (char*)data, w, h, 8, 0);
	}

	row_data = new unsigned char*[h];
	for(int i = 0; i < h; i++)
	{
		row_data[i] = &data[i * ximage->bytes_per_line];
	}
// This differs from the depth parameter of the top_level.
	bits_per_pixel = ximage->bits_per_pixel;
return 0;
}

int BC_Capture::delete_data()
{
	if(data)
	{
		if(use_shm)
		{
			XShmDetach(display, &shm_info);
			XDestroyImage(ximage);
			shmdt(shm_info.shmaddr);
		}
		else
		{
			XDestroyImage(ximage);
		}

// data is automatically freed by XDestroyImage
		data = 0;
		delete row_data;
	}
return 0;
}

int BC_Capture::get_top_w()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return WidthOfScreen(screen_ptr);
return 0;
}

int BC_Capture::get_top_h()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return HeightOfScreen(screen_ptr);
return 0;
}

BC_ImportPixel::BC_ImportPixel(int depth, int byte_swap)
{
	switch(depth)
	{
		case 8:
			if(byte_swap) importer = new BC_ImportPixel8Swap;
			else importer = new BC_ImportPixel8;
			break;
		case 16:
			if(byte_swap) importer = new BC_ImportPixel16Swap;
			else importer = new BC_ImportPixel16;
			break;
		case 24:
			if(byte_swap) importer = new BC_ImportPixel24Swap;
			else importer = new BC_ImportPixel24;
			break;
		case 32:
			if(byte_swap) importer = new BC_ImportPixel32Swap;
			else importer = new BC_ImportPixel32;
			break;
		default:
			importer = new BC_ImportPixel8;
			break;
	}
}

BC_ImportPixel::~BC_ImportPixel()
{
}

int BC_ImportPixel::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	importer->import_pixel(output, input);
return 0;
}


BC_ImportPixelBase::BC_ImportPixelBase()
{
}
BC_ImportPixelBase::~BC_ImportPixelBase()
{
}



BC_ImportPixel8::BC_ImportPixel8() {}
int BC_ImportPixel8::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	output.r = 0;
	output.g = 0;
	output.b = 0;
	(*input)++;
return 0;
}

BC_ImportPixel8Swap::BC_ImportPixel8Swap() {}
int BC_ImportPixel8Swap::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	output.r = 0;
	output.g = 0;
	output.b = 0;
	(*input)++;
return 0;
}


BC_ImportPixel16::BC_ImportPixel16() {}
int BC_ImportPixel16::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	temp = *((unsigned TWO*)*input);
	(*input) += 2;

	output.r = (temp & 0xf800) >> 8;
	output.g = (temp & 0x7e0) >> 3;
	output.b = (temp & 0x1f) << 3;

	(*input)++;
return 0;
}

BC_ImportPixel16Swap::BC_ImportPixel16Swap() {}
int BC_ImportPixel16Swap::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	temp = *((unsigned TWO*)*input);
	(*input) += 2;

	output.r = (temp & 0xf800) >> 8;
	output.g = (temp & 0x7e0) >> 3;
	output.b = (temp & 0x1f) << 3;
return 0;
}


BC_ImportPixel24::BC_ImportPixel24() {}
int BC_ImportPixel24::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	output.r = *(*input)++;
	output.g = *(*input)++;
	output.b = *(*input)++;
return 0;
}

BC_ImportPixel24Swap::BC_ImportPixel24Swap() {}
int BC_ImportPixel24Swap::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	output.b = *(*input)++;
	output.g = *(*input)++;
	output.r = *(*input)++;
return 0;
}


BC_ImportPixel32::BC_ImportPixel32() {}
int BC_ImportPixel32::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	(*input)++;
	output.b = *(*input)++;
	output.g = *(*input)++;
	output.r = *(*input)++;
return 0;
}

// 24 bpp unpacked:       0bgr
BC_ImportPixel32Swap::BC_ImportPixel32Swap() {}
int BC_ImportPixel32Swap::import_pixel(BC_CapturePixel &output, unsigned char **input)
{
	output.r = (*input)[2];
	output.g = (*input)[1];
	output.b = (*input)[0];
	(*input) += 4;
return 0;
}
