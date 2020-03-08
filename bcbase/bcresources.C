#include <string.h>

#include "bcbitmap.h"
#include "bccolors.h"
#include "bcipc.h"
#include "bcresources.h"
#include "bcwindow.h"
#include "colormodels.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

int BC_Resources::error = 0;

int BC_Resources::error_handler(Display *display, XErrorEvent *event)
{
	error = 1;
	return 0;
return 0;
}

BC_Resources::BC_Resources(BC_Window *window)
{
	use_shm = 1;
	use_yuv = 0;

// Set up IPC cleanup handlers
	bc_init_ipc();

// Test for shm.  Must come before yuv test
	init_shm(window);

// Test for yuv
	init_yuv(window);

// Initialize
	bg_color = MEGREY;
	bg_shadow1 = DKGREY;
	bg_shadow2 = BLACK;
	bg_light1 = WHITE;
	bg_light2 = bg_color;

	button_light = WHITE;           // bright corner
	button_highlighted = LTGREY;  // face when highlighted
	button_down = MDGREY;         // face when down
	button_up = MEGREY;           // face when up
	button_shadow = DKGREY;       // dark corner

	menu_light = LTCYAN;
	menu_highlighted = LTBLUE;
	menu_down = MDCYAN;
	menu_up = MECYAN;
	menu_shadow = DKCYAN;

	text_default = BLACK;
	text_background = WHITE;
	highlight_inverse = WHITE ^ BLUE;

	double_click = 300;

	//sprintf(large_font, "-*-charter-*-r-*-*-24-*-*-*-*-*-*-*");
	//sprintf(large_font, "-*-helvetica-bold-r-normal--24-240-75-75-p-124-iso8859-1");
	sprintf(medium_font, "-*-helvetica-bold-r-normal-*-14-*");
	sprintf(small_font, "-*-helvetica-medium-r-normal-*-10-*");
	sprintf(large_font, "-*-helvetica-bold-r-normal-*-18-*");
}

BC_Resources::~BC_Resources()
{
}

int BC_Resources::init_shm(BC_Window *window)
{
	XSetErrorHandler(BC_Resources::error_handler);
	if(!XShmQueryExtension(window->display)) use_shm = 0;
	else
	{
		XShmSegmentInfo test_shm;
		XImage *test_image;
		unsigned char *data;
		test_image = XShmCreateImage(window->display, window->vis, window->depth,
                ZPixmap, (char*)NULL, &test_shm, 5, 5);

		test_shm.shmid = shmget(IPC_PRIVATE, 5 * test_image->bytes_per_line, (IPC_CREAT | 0777 ));
		data = (unsigned char *)shmat(test_shm.shmid, NULL, 0);
		BC_Resources::error = 0;
 	   	XShmAttach(window->display, &test_shm);
    	XSync(window->display, False);
		if(BC_Resources::error) use_shm = 0;
		XDestroyImage(test_image);
		shmdt(test_shm.shmaddr);
    	shmctl(test_shm.shmid, IPC_RMID, 0);
	}
	return 0;
return 0;
}

int BC_Resources::init_yuv(BC_Window *window)
{
	yuv_portid = get_portid(window, FOURCC_YV12);
	if(yuv_portid >= 0)
	{
		use_yuv = 1; 
	}
	else
	{
		use_yuv = 0;
		yuv_portid = 0;
	}
//use_yuv = 0;

	return 0;
return 0;
}


int BC_Resources::get_portid(BC_Window *window, int color_model)
{
	int numFormats, i, j;
	unsigned int ver, rev, numAdapt, reqBase, eventBase, errorBase;
	int portid = -1;
    XvAdaptorInfo *info;
    XvImageFormatValues *formats;

// Only local server is fast enough.
	if(!use_shm) return portid;

// XV extension is available
    if(Success != XvQueryExtension(window->display, 
				  &ver, 
				  &rev, 
				  &reqBase, 
				  &eventBase, 
				  &errorBase))
    {
		return portid;
    }

// XV adaptors are available
	XvQueryAdaptors(window->display, 
		DefaultRootWindow(window->display), 
		&numAdapt, 
		&info);

	if(!numAdapt)
	{
		return portid;
	}

// Get adaptor with desired color model
    for(i = 0; (i < numAdapt) && (portid == -1); i++)
    {
		if(info[i].type & XvImageMask) /* adaptor supports XvImages */
		{  
	    	formats = XvListImageFormats(window->display, 
							info[i].base_id, 
							&numFormats);
	    	for(j = 0; j < numFormats; j++) 
	    	{
				if(formats[j].id == FOURCC_YV12 && color_model == BC_YUV420P)
				{
/* this adaptor supports YUV 420 */
		    		portid = info[i].base_id;
		    		break;
				}
			}
	    	if(formats) XFree(formats);
		}
	}

    XFree(info);

	return portid;
return 0;
}

int BC_Resources::get_bg_color() { return bg_color; return 0;
}

int BC_Resources::get_bg_shadow1() { return bg_shadow1; return 0;
}

int BC_Resources::get_bg_shadow2() { return bg_shadow2; return 0;
}

int BC_Resources::get_bg_light1() { return bg_light1; return 0;
}

int BC_Resources::get_bg_light2() { return bg_light2; return 0;
}

