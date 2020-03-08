#include "bcdisplayinfo.h"


BC_DisplayInfo::BC_DisplayInfo(char *display_name)
{
	init_window(display_name);
}

BC_DisplayInfo::~BC_DisplayInfo()
{
	XCloseDisplay(display);
}


void BC_DisplayInfo::parse_geometry(char *geom, int *x, int *y, int *width, int *height)
{
	XParseGeometry(geom, x, y, (unsigned int*)width, (unsigned int*)height);
}

void BC_DisplayInfo::init_window(char *display_name)
{
	if(display_name && display_name[0] == 0) display_name = NULL;
	if((display = XOpenDisplay(display_name)) == NULL)
	{
  		printf("cannot connect to X server.\n");
  		if(getenv("DISPLAY") == NULL)
    		printf("'DISPLAY' environment variable not set.\n");
  		exit(-1);
 	}
	
	screen = DefaultScreen(display);
	rootwin = RootWindow(display, screen);
	vis = DefaultVisual(display, screen);
}


int BC_DisplayInfo::get_root_w()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return WidthOfScreen(screen_ptr);
}

int BC_DisplayInfo::get_root_h()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return HeightOfScreen(screen_ptr);
}

int BC_DisplayInfo::get_abs_cursor_x()
{
	int abs_x, abs_y, win_x, win_y;
	unsigned int temp_mask;
	Window temp_win;

	XQueryPointer(display, 
	   rootwin, 
	   &temp_win, 
	   &temp_win,
       &abs_x, 
	   &abs_y, 
	   &win_x, 
	   &win_y, 
	   &temp_mask);
	return abs_x;
}

int BC_DisplayInfo::get_abs_cursor_y()
{
	int abs_x, abs_y, win_x, win_y;
	unsigned int temp_mask;
	Window temp_win;

	XQueryPointer(display, 
	   rootwin, 
	   &temp_win, 
	   &temp_win,
       &abs_x, 
	   &abs_y, 
	   &win_x, 
	   &win_y, 
	   &temp_mask);
	return abs_y;
}
