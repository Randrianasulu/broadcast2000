#ifndef BCDISPLAYINFO_H
#define BCDISPLAYINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

class BC_DisplayInfo
{
public:
	BC_DisplayInfo(char *display_name = "");
	~BC_DisplayInfo();
	
	void init_window(char *display_name);
	int get_root_w();
	int get_root_h();
	int get_abs_cursor_x();
	int get_abs_cursor_y();
	static void parse_geometry(char *geom, int *x, int *y, int *width, int *height);


private:
	Display* display;
	Window rootwin;
	Visual *vis;
	int screen;
};

#endif
