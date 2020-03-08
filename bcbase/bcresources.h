#ifndef BCRESOURCES
#define BCRESOURCES

#include <X11/Xlib.h>
#include "bcwindow.inc"

class BC_Resources
{
public:
	BC_Resources(BC_Window *window); // The window parameter is used to get the display information initially
	~BC_Resources();

	int get_bg_color();          // window backgrounds
	int get_bg_shadow1();        // border for windows
	int get_bg_shadow2();
	int get_bg_light1();
	int get_bg_light2();


// colors
	int bg_color;          // window backgrounds
	int bg_shadow1;        // border for windows
	int bg_shadow2;
	int bg_light1;
	int bg_light2;


// these are used for everything
	int button_light;      
	int button_highlighted;
	int button_down;       
	int button_up;         
	int button_shadow;     

// highlighting
	int highlight_inverse;

// for menus
	int menu_light;
	int menu_highlighted;
	int menu_down;
	int menu_up;
	int menu_shadow;

// ms for double click
	long double_click;

	int text_default;      // default color of text
	int text_background;   // background color of textboxes and list boxes

// fonts
	char large_font[1024], medium_font[1024], small_font[1024];

// Available display extensions
	int use_shm;
	int use_yuv;
	int yuv_portid;     // Port ID for the XVideo extension
	
	static int error;

private:
// Test for availability of shared memory pixmaps
	int init_shm(BC_Window *window);
	int init_yuv(BC_Window *window);
	static int error_handler(Display *display, XErrorEvent *event);

// Get the port ID for a color model or return -1 for failure
	int get_portid(BC_Window *window, int color_model);
};


#endif
