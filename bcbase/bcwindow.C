#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "bcbitmap.h"
#include "bccolors.h"
#include "bcfont.h"
#include "bckeys.h"
#include "bcmenubar.h"
#include "bcpopupmenu.h"
#include "bcrepeater.h"
#include "bcresources.h"
#include "bcwindow.h"
#include "byteorder.h"

BC_Resources* BC_Window::resources = 0;

BC_Window::BC_Window(const char *title, 
			int w, 
			int h, 
			int minw, 
			int minh, 
			int private_color, 
			int hide)
 : BC_WindowBase(0, 0, w, h)
{
	char *display_name = NULL;
	init_flags();
	init_window(display_name, title, hide, private_color, minw, minh);
}

BC_Window::BC_Window(const char *display_name, 
					 int color, 
					 const char *title, 
					 int w, int h, 
					 int minw, int minh, 
					 int private_color, int hide)
 : BC_WindowBase(0, 0, w, h, color)
{
	init_flags();
	init_window(display_name, title, hide, private_color, minw, minh);
}

BC_Window::~BC_Window()
{         // base class deletes everything since it's the last destructor called
//	if(resources) delete resources;
	unset_all_repeaters();
}

int BC_Window::init_flags()
{
	ctrl_mask = shift_mask = button_pressed = key_pressed = key_sym = resized = 0;
	done = 0;
	button_down = double_click = cursorleft = button_just_released = 0;
	button_time1 = button_time2 = 0;
	motion_buffer = 0;
	last_deleted_window = motion_buffer_win = 0;
	active_menubar = 0;
	active_tool = 0;
	active_popup_menu = 0;
	last_w = w;
	last_h = h;
	next_repeat_id = 0;
return 0;
}

int BC_Window::init_window(const char *display_name, 
		const char *title, 
		int hide, 
		int private_color, 
		int minw, 
		int minh)
{
	this->hidden = hide;
	this->private_color = private_color;

// This function must be the first Xlib
// function a multi-threaded program calls
	XInitThreads();

// get the display name
	if(display_name && display_name[0] == 0) display_name = NULL;
	if((display = XOpenDisplay(display_name)) == NULL)
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
	client_byte_order = get_byte_order();
	server_byte_order = XImageByteOrder(display);
	if(server_byte_order == MSBFirst)
		server_byte_order = 0;
	else
		server_byte_order = 1;

	init_colors();

	if(!resources) resources = new BC_Resources(this);
	if(color == -1)
		color = resources->get_bg_color();

	init_fonts();
	init_gc();

	Cursor arrow;
	arrow = XCreateFontCursor(display, XC_top_left_arrow);

	XSetWindowAttributes attr;
	unsigned long mask;
	mask = CWBackingStore | CWEventMask | CWBackPixel | CWBorderPixel | CWColormap | CWCursor | CWOverrideRedirect | CWSaveUnder;
	attr.event_mask = LeaveWindowMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | KeyPressMask | PointerMotionMask | FocusChangeMask;
	attr.background_pixel = get_color(color);
	attr.border_pixel = get_color(color);
	attr.colormap = cmap;
	attr.cursor = arrow;
	attr.override_redirect = False;
	attr.save_under = False;
	attr.backing_store = Always;

	win = XCreateWindow(display, rootwin, 0, 0, w, h, 0, depth, InputOutput, vis, mask, &attr);

	XSizeHints size_hints;
	size_hints.flags = PSize | PMinSize | PMaxSize;
	size_hints.width = w;
	size_hints.height = h;
	size_hints.min_width = minw;
// Enlightenment fix
//	if(minw == w) size_hints.max_width = w; else size_hints.max_width = 32767;
	size_hints.max_width = 32767;
	size_hints.min_height = minh;
//	if(minh == h) size_hints.max_height = h; else size_hints.max_height = 32767;
	size_hints.max_height = 32767;

	XSetStandardProperties(display, win, title, title, None, 0, 0, &size_hints);

	get_atoms();
	add_window(win);
	if(!hide) show_window();

	XFlush(display);
return 0;
}

int BC_Window::get_atoms()
{
	RepeaterXAtom = XInternAtom(display, "BC_REPEAT_EVENT", False);
//printf("BC_Window::get_atoms %ld\n", RepeaterXAtom);
	DelWinXAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	if((ProtoXAtom = XInternAtom(display, "WM_PROTOCOLS", False)))
		XChangeProperty(display, win, ProtoXAtom, XA_ATOM, 32, PropModeReplace, (unsigned char *)&DelWinXAtom, True );
return 0;
}


int BC_Window::init_gc()
{
	unsigned long gcmask;
	gcmask = GCFont | GCGraphicsExposures;

	XGCValues gcvalues;
	gcvalues.font = mediumfont->fid;        // set the font
	gcvalues.graphics_exposures = 0;        // prevent expose events for every redraw
	gc = XCreateGC(display, rootwin, gcmask, &gcvalues);
return 0;
}

int BC_Window::init_fonts()
{
	if((largefont = XLoadQueryFont(display, resources->large_font)) == NULL) 
	{
		largefont = XLoadQueryFont(display, "fixed"); 
	}

	if((mediumfont = XLoadQueryFont(display, resources->medium_font)) == NULL)
	{ 
		mediumfont = XLoadQueryFont(display, "fixed"); 
	}

	if((smallfont = XLoadQueryFont(display, resources->small_font)) == NULL)
	{ 
		smallfont = XLoadQueryFont(display, "fixed"); 
	}
return 0;
}


int BC_Window::init_colors()
{
	total_colors = 0;
	current_color_value = current_color_pixel = 0;
	
	depth = DefaultDepth(display, screen);

	switch(depth)
	{
		case 8:
			if(private_color)
			{
  				cmap = XCreateColormap(display, rootwin, vis, AllocNone);
				create_private_colors();
			}
			else
			{
	 		  	cmap = DefaultColormap(display, screen);
				create_shared_colors();
			}

			allocate_color_table();
			break;

		default:
 			cmap = DefaultColormap(display, screen);
			break;
	}
return 0;
}

int BC_Window::create_private_colors()
{
	int color;
	total_colors = 256;

	for(int i = 0; i < 255; i++)
	{
		color = (i & 0xc0) << 16;
		color += (i & 0x38) << 10;
		color += (i & 0x7) << 5;
		color_table[i][0] = color;
	}
	create_shared_colors();        // overwrite the necessary colors on the table
return 0;
}


int BC_Window::create_color(int color)
{
	if(total_colors == 256)
	{
// replace the closest match with an exact match
		color_table[get_color_8(color)][0] = color;
	}
	else
	{
// add the color to the table
		color_table[total_colors][0] = color;
		total_colors++;
	}
return 0;
}

int BC_Window::create_shared_colors()
{
	create_color(BLACK);
	create_color(WHITE);   

	create_color(LTGREY);  
	create_color(MEGREY);  
	create_color(MDGREY);  
	create_color(DKGREY);   		  	

	create_color(LTCYAN);  
	create_color(MECYAN);  
	create_color(MDCYAN);  
	create_color(DKCYAN);  

	create_color(LTGREEN); 
	create_color(GREEN);   
	create_color(DKGREEN); 

	create_color(LTPINK);  
	create_color(PINK);
	create_color(RED);     

	create_color(LTBLUE);  
	create_color(BLUE);    
	create_color(DKBLUE);  

	create_color(LTYELLOW); 
	create_color(MEYELLOW); 
	create_color(MDYELLOW); 
	create_color(DKYELLOW); 

	create_color(LTPURPLE); 
	create_color(MEPURPLE); 
	create_color(MDPURPLE); 
	create_color(DKPURPLE); 
return 0;
}

int BC_Window::allocate_color_table()
{
	int red, green, blue, color;
	int result;
	XColor col;

	for(int i = 0; i < total_colors; i++)
	{
		color = color_table[i][0];
		red = (color & 0xFF0000) >> 16;
		green = (color & 0x00FF00) >> 8;
		blue = color & 0xFF;

		col.flags = DoRed | DoGreen | DoBlue;
		col.red   = red<<8   | red;
		col.green = green<<8 | green;
		col.blue  = blue<<8  | blue;

		XAllocColor(display, cmap, &col);
		color_table[i][1] = col.pixel;

//printf("BC_Window::allocate_color_table %x %x %x %d\n", col.red, col.green, col.blue, color_table[i][1]);
		//if(!)
		//{
		//	cmap = XCopyColormapAndFree(display, cmap);
		//	XAllocColor(display, cmap, &Col);
		//}
	}

	XInstallColormap(display, cmap);
return 0;
}

int BC_Window::get_color(long color) 
{
// return pixel of color
// use this only for drawing tools not for bitmaps
	 int i, test, difference, result;
		
	if(depth <= 8)
	{
		if(private_color)
		{
			return get_color_8(color);
		}
		else
		{
// test last color looked up
			if(current_color_value == color) return current_color_pixel;

// look up in table
			current_color_value = color;
			for(i = 0; i < total_colors; i++)
			{
				if(color_table[i][0] == color)
				{
					current_color_pixel = color_table[i][1];
//printf("BC_Window::get_color %x %d\n", color, current_color_pixel);
					return current_color_pixel;
				}
			}

// find nearest match
			difference = 0xFFFFFF;

			for(i = 0, result = 0; i < total_colors; i++)
			{
				test = abs((int)(color_table[i][0] - color));

				if(test < difference) 
				{
					current_color_pixel = color_table[i][1]; 
					difference = test;
				}
			}

			return current_color_pixel;
		}
	}	
	else
	switch(depth){
		case 16:
			if(client_byte_order == server_byte_order) return get_color_16(color);
			else
			return get_color_16_swap(color);
			break;
		case 24:
			if(client_byte_order == server_byte_order) return color;
			else
			return get_color_24_swap(color);
			break;
		default:
			return color;
			break;	
	}
	return color;
return 0;
}

int BC_Window::get_color_8(int color)
{
	int pixel;

	pixel = (color & 0xc00000) >> 16;
	pixel += (color & 0xe000) >> 10;
	pixel += (color & 0xe0) >> 5;
	return pixel;
return 0;
}

long BC_Window::get_color_16(int color)
{
	long result;
	result = (color & 0xf80000) >> 8;
	result += (color & 0xfc00) >> 5;
	result += (color & 0xf8) >> 3;
	
	return result;
}

long BC_Window::get_color_16_swap(int color)
{
	long result;
	result = (color & 0xf80000) >> 19;
	result += (color & 0xfc00) >> 5;
	result += (color & 0xf8) << 8;

	return result;
}

long BC_Window::get_color_24_swap(int color)
{
	long result;
	result = (color & 0xff) << 16;
	result += (color & 0xff00);
	result += (color & 0xff0000) >> 16;
	return result;
}

XFontStruct* BC_Window::get_font(int font)
{
	switch(font)
	{
		case MEDIUMFONT: return mediumfont; break;
		case SMALLFONT: return smallfont; break;
		case LARGEFONT: return largefont; break;
	}
return 0;
}

int BC_Window::set_font(int font)
{
	switch(font)
	{
		case LARGEFONT: XSetFont(display, gc, largefont->fid); break;
		case SMALLFONT: XSetFont(display, gc, smallfont->fid); break;
		case MEDIUMFONT: XSetFont(display, gc, mediumfont->fid); break;
	}
return 0;
}

// =========================== initialization

BC_Tool* BC_Window::add_tool(BC_Tool *tool)
{
	if(!top_level) top_level = this;
	BC_WindowBase::add_tool(tool);
return 0;
}

int BC_Window::add_border(int light, int medium, int dark)
{
	if(!top_level) top_level = this;
	BC_WindowBase::add_border(light, medium, dark);
return 0;
}

BC_SubWindow* BC_Window::add_subwindow(BC_SubWindow* subwindow)
{
	if(!top_level) top_level = this;
	BC_WindowBase::add_subwindow(subwindow);
return 0;
}





// =========================== event dispatching



int BC_Window::run_window()
{
// initialize final flags
	done = 0;             // reset done flag
	return_value = 0;     // default return value
	top_level = this;
	parent_window = this;

	while(!done)
	{
		dispatch_event();
	}

	unlock_window();
	return return_value;
return 0;
}

int BC_Window::get_key_masks(XEvent &report)
{
// ctrl key down
		if(report.xkey.state & ControlMask) ctrl_mask = 1;
// shift key down
		if(report.xkey.state & ShiftMask) shift_mask = 1;
return 0;
}

int BC_Window::dispatch_event()
{
   XEvent report, report2;
   Window tempwin;
   int repeat_next_event;     // for clearing motion notifies
   int button_x, button_y;
   int result, i;         // result from a dispatch determines if event has been trapped

// zero all flags
	key_pressed = 0;
	button_pressed = 0;
	ctrl_mask = 0;
	shift_mask = 0;
	cursorleft = 0;
	resized = 0;
	event_win = 0;
	double_click = 0;
	new_repeater = 0;
	report.type = 0;           // clear this report in case next event is a repeat or motion buffer

// If an event is waiting get it, otherwise
// wait for next event only if there is no motion history.
	if(XPending(display) || (!motion_buffer))
	{
		XNextEvent(display, &report);

// Lock out anyone who tries to delete a window beyond this point.
// Deletions between XNextEvent and this get through of course.
		lock_window();
	}
	else
	if(motion_buffer)
	{
// Handle buffered motion events if there is no waiting event
		lock_window();
		get_key_masks(report);
		dispatch_motion_event_main();
	}

	get_key_masks(report);

// Get cursor position for button events relative to top_level
	if((report.type == ButtonPress || report.type == ButtonRelease) &&
		window_exists(report.xany.window))
	{
		XTranslateCoordinates(display, report.xany.window, win, report.xbutton.x, report.xbutton.y, &button_x, &button_y, &tempwin);
	}

	switch(report.type)
	{
		case Expose:
			event_win = report.xany.window;
			expose_event_dispatch();
			break;
			
		case ClientMessage:
// clear the motion buffer since this can clear the window
			if(motion_buffer) { dispatch_motion_event_main(); }

			{
				XClientMessageEvent *ptr;
				ptr = (XClientMessageEvent*)&report;

//printf("ClientMessage %ld %ld\n", ptr->message_type, ptr->data.l[0]);
        		if(ptr->message_type == ProtoXAtom && 
        			ptr->data.l[0] == DelWinXAtom)
        		{
					close_event();
				}
				else
				if(ptr->message_type == RepeaterXAtom)
				{
// Make sure the repeater still exists.
					for(i = 0; i < repeaters.total; i++)
					{
						if(repeaters.values[i]->repeat_id == ptr->data.l[0])
						{
							repeat_event_dispatch(ptr->data.l[0], ptr->data.l[1]);
							i = repeaters.total;
						}
					}
				}
			}
			break;
		case ButtonPress:
// get location information
			cursor_x = button_x;
			cursor_y = button_y;

// get button information
			button_pressed = report.xbutton.button;

// get time information
			event_win = report.xany.window;
  			button_down = 1;
			button_time1 = button_time2;
			button_time2 = report.xbutton.time;
			if(button_time2 - button_time1 < resources->double_click)
			{
// Don't want triple clicks
				double_click = 1; 
				button_time2 = button_time1 = 0; 
			}
			else 
				double_click = 0;

			if(active_menubar)
				active_menubar->button_press_dispatch();
			else
			if(active_popup_menu)
				active_popup_menu->button_press_dispatch();
			else
				button_press_dispatch();
			break;
		case ButtonRelease:
// get location information
			cursor_x = button_x;
			cursor_y = button_y;

			event_win = report.xany.window;
			button_just_released++;   // X generates cursor leave events after button release
  			button_down = 0;

			if(active_menubar) active_menubar->button_release_dispatch();
			else
			if(active_popup_menu) active_popup_menu->button_release_dispatch();
  			else 
  			button_release_dispatch();
			break;
		case MotionNotify:
// Buffer it if it's the first motion from any window.
// Window can get destroyed after the button release and the motion notify event.
			if(!motion_buffer)
			{
				store_motion_event(&report);
			}
			else
// buffer it if it's a subsequent motion from the same window
			if(motion_buffer_win == report.xany.window)
			{
				store_motion_event(&report);
			}
			else
// dispatch it if it's a subsequent motion from a different window			
			{
				dispatch_motion_event_main();
				
// arm buffer with new motion event
				store_motion_event(&report);
			}
			break;
		case ConfigureNotify:
		{
			int cancel_resize = 0;
			for(int i = 0; i < resize_history.total && !cancel_resize; i++)
			{
				if(resize_history.values[i]->w == report.xconfigure.width &&
					resize_history.values[i]->h == report.xconfigure.height)
				{
					delete resize_history.values[i];
					resize_history.remove_number(i);
					cancel_resize = 1;
				}
			}

			if(!cancel_resize &&
				report.xany.window == win && 
				(report.xconfigure.width != w || report.xconfigure.height != h)
				&& (report.xconfigure.width != last_w || report.xconfigure.height != last_h))
			{
				last_w = w = report.xconfigure.width;
				last_h = h = report.xconfigure.height;
				resized = 1;

// only give to user here
				resize_event(w, h);
// send to all objects
				resize_event_dispatch();
			}
		}
			break;
		case KeyPress:
  			KeySym keysym;
  			char keys_return[2];
			key_pressed = 0;

  			keys_return[0] = '\0';
  			XLookupString ((XKeyEvent*)&report, keys_return, 1, &keysym, 0);

// block out control keys
			if(keysym > 0xffe0 && keysym < 0xffff) { break; }

  			switch(keysym){
// block out some keys
				case XK_Return:     key_pressed = 13;        break;
        		case XK_Alt_L:      key_pressed = 0;         break;
        		case XK_Alt_R:      key_pressed = 0;         break;
        		case XK_Shift_L:    key_pressed = 0;         break;
        		case XK_Shift_R:    key_pressed = 0;         break;
        		case XK_Control_L:  key_pressed = 0;         break;
        		case XK_Control_R:  key_pressed = 0;         break;

// translate key codes
  	    		case XK_Up:         key_pressed = UP;        break;
   				case XK_Down:       key_pressed = DOWN;      break;
   				case XK_Left:       key_pressed = LEFT;      break;
    			case XK_Right:      key_pressed = RIGHT;     break;
    			case XK_Next:       key_pressed = PGDN;      break;
    			case XK_Prior:      key_pressed = PGUP;      break;
    			case XK_BackSpace:  key_pressed = BACKSPACE; break;
  	    		case XK_Escape:     key_pressed = ESC;       break;
  	    		case XK_Tab:        key_pressed = TAB;       break;
 				case XK_underscore: key_pressed = '_';       break;
   	    		case XK_asciitilde: key_pressed = '~';       break;
				case XK_Delete:     key_pressed = DELETE;     break;

// number pad
				case XK_KP_Enter:       key_pressed = KPENTER;   break;
				case XK_KP_Add:         key_pressed = KPPLUS;    break;
				case XK_KP_1:
				case XK_KP_End:         key_pressed = KP1;       break;
				case XK_KP_2:
				case XK_KP_Down:        key_pressed = KP2;       break;
				case XK_KP_3:
				case XK_KP_Page_Down:   key_pressed = KP3;       break;
				case XK_KP_4:
				case XK_KP_Left:        key_pressed = KP4;       break;
				case XK_KP_5:
				case XK_KP_Begin:       key_pressed = KP5;       break;
				case XK_KP_6:
				case XK_KP_Right:       key_pressed = KP6;       break;
				case XK_KP_0:
				case XK_KP_Insert:      key_pressed = KPINS;     break;
				case XK_KP_Decimal:
				case XK_KP_Delete:      key_pressed = KPDEL;     break;

 	    		default:       if(!(keysym & 0xff00)) key_pressed = keys_return[0]; break;
  			}

// dispatch keypress until trapped
			if(key_pressed > 0)
			{
				result = keypress_event_dispatch();

// give to user after everyone else has trapped it		
  				if(key_pressed && enabled && !result) result = keypress_event();    
			}
			break;

		case FocusOut:
			cursor_x = cursor_y = -1;
			cursor_left_dispatch();
			break;

		case LeaveNotify:
			event_win = report.xany.window;
			if(report.xcrossing.focus)
			{
// button release dispatches leavenotify
				cursor_x = report.xcrossing.x;
				cursor_y = report.xcrossing.y;
			}
			else
			{
// cursor position is reported inside tool when crossing to a higher window
				cursor_x = cursor_y = -1;
			}

// dispatch to subwindows
			cursor_left_dispatch();
			break;
//		case 65:
//			printf("BC_Window recieved a ShmCompletion.  Shit!\n");
//			shm_completions++;
//			break;
	}

	unlock_window();
return 0;
}


int BC_Window::pick_event(int event_type)
{
	XEvent report;
	do
	{
		XNextEvent(display, &report);
		if(report.type != event_type) event_queue.append(report);
	}while(report.type != event_type);

	for(int i = event_queue.total - 1; i >= 0; i--)
	{
		XPutBackEvent(display, &(event_queue.values[i]));
	}

	event_queue.remove_all();
	return 0;
return 0;
}






// ================================== window registry

int BC_Window::add_window(Window win)
{
	if(!window_exists(win)) window_registry.append(win);
return 0;
}

int BC_Window::delete_window(Window win)
{
	window_registry.remove(win);
return 0;
}

int BC_Window::window_exists(Window win)
{
	for(int i = 0; i < window_registry.total; i++)
	{
		if(window_registry.values[i] == win) return 1;
	}
	return 0;
return 0;
}




// ================================== event handlers

int BC_Window::store_motion_event(XEvent *report)
{
	Window tempwin;

// only store if the motion wasn't in the last deleted window
	if(last_deleted_window != report->xany.window &&
		window_exists(report->xany.window))
	{
		motion_buffer = 1;
		XTranslateCoordinates(display, report->xany.window, win, report->xmotion.x, report->xmotion.y, &motion_buffer_x, &motion_buffer_y, &tempwin);
		motion_buffer_win = report->xany.window;
	}
return 0;
}

int BC_Window::dispatch_motion_event_main()
{
	int result = 0;
	event_win = motion_buffer_win;
	cursor_x = motion_buffer_x;
	cursor_y = motion_buffer_y;
	motion_buffer = 0;

	if(active_menubar) result = active_menubar->motion_event_dispatch();
	if(active_popup_menu && !result) result = active_popup_menu->motion_event_dispatch();
	if(active_tool && !result) result = active_tool->motion_event_dispatch();
	if(!result) result = motion_event_dispatch();

	return result;
return 0;
}





// ===================================== configuration

int BC_Window::set_last_deleted(Window win) { last_deleted_window = win; return 0;
}

int BC_Window::shift_down(){ return shift_mask; return 0;
};
int BC_Window::ctrl_down(){ return ctrl_mask; return 0;
};

int BC_Window::show_window() { XMapWindow(display, win); hidden = 0; XFlush(display); return 0;
}
int BC_Window::hide_window() { XUnmapWindow(display, win); hidden = 1; XFlush(display); return 0;
}

// flush out all the events
int BC_Window::window_reset()
{
	XEvent report;
	
	//while(XPending(display)) XNextEvent(display, &report);
	//flash();
return 0;
}


int BC_Window::lock_window() 
{ 
	XLockDisplay(display); 
return 0;
}
int BC_Window::unlock_window() 
{ 
	XUnlockDisplay(display); 
return 0;
}






// ================================== repeating

int BC_Window::repeat_event_dispatch(long repeat_id, int interrupt_now)
{
	int result = 0;
	BC_Repeater *repeater;

// Unlock the repeater if it still exists.
	for(int i = 0; i < repeaters.total; i++)
	{
		if(repeaters.values[i]->repeat_id == repeat_id)
		{
			repeater = repeaters.values[i];
			if(repeater->interrupted)
			{
// Disregard
				if(interrupt_now)
				{
// Delete now
					repeater->join();
					repeaters.remove(repeater);
					delete repeater;
				}
			}
			else
			{
				if(active_menubar) result = active_menubar->repeat_event_dispatch(repeat_id);
				if(!result && active_tool) result = active_tool->repeat_event_dispatch(repeat_id);
				if(!result) BC_WindowBase::repeat_event_dispatch(repeat_id);

				repeater->repeat_mutex.unlock();
			}
			i = repeaters.total;
		}
	}
	return result;
return 0;
}

// signal the event handler to repeat
int BC_Window::set_repeat(long duration, long id)                  
{
// test repeater database
	for(int i = 0; i < repeaters.total; i++)
	{
		if(repeaters.values[i]->repeat_id == id)
		{
			repeaters.values[i]->delay = duration;
			return 0;
		}
	}

	BC_Repeater *repeater = new BC_Repeater(this, id, duration);
	repeaters.append(repeater);

    repeater->start_repeating();
	return 0;
return 0;
}

int BC_Window::unset_repeat(long id)
{
	BC_Repeater *repeater = 0;
	for(int i = 0; i < repeaters.total; i++)
	{
		if(repeaters.values[i]->repeat_id == id)
		{
			repeater = repeaters.values[i];
			i = repeaters.total;
		}
	}

	if(repeater)
	{
		repeater->stop_repeating();
//		repeaters.remove(repeater);
//		delete repeater;
	}
return 0;
}

int BC_Window::unset_all_repeaters()
{
	for(int i = 0; i < repeaters.total; i++)
	{
		repeaters.values[i]->stop_repeating();
		delete repeaters.values[i];
	}
	repeaters.remove_all();
return 0;
}

long BC_Window::get_repeat(long repeat_id)
{ 
	for(int i = 0; i < repeaters.total; i++)
	{
		if(repeaters.values[i]->repeat_id == repeat_id)
		{
			return repeaters.values[i]->delay;
		}
	}
	return 0;
}

long BC_Window::new_repeat_id()
{
	return next_repeat_id++;
}

int BC_Window::arm_repeat(long repeat_id, int interrupt_now)
{
	lock_window();
	XEvent report;
	XClientMessageEvent *ptr = (XClientMessageEvent*)&report;
	ptr->type = ClientMessage;
	ptr->message_type = RepeaterXAtom;
	ptr->format = 32;
	ptr->data.l[0] = repeat_id;
	ptr->data.l[1] = interrupt_now;

	XSendEvent(display, win, 0, 0, &report);
	XFlush(display);
	unlock_window();
	return 0;
return 0;
}






// ================================== user configuration

int BC_Window::set_done(int return_value)
{
	XExposeEvent report;
	report.type = Expose;
	done = 1;
	XSendEvent(display, win, 0, 0, (XEvent *)&report);
	this->return_value = return_value;
	XFlush(display);
return 0;
}

int BC_Window::set_title(const char *title) 
{ 
	XSetStandardProperties(display, win, title, title, None, 0, 0, 0); 
	XFlush(display);
return 0;
}

// ====================================== clipboard

int BC_Window::to_clipboard(char *data)
{
// Bad match error
	//XRotateBuffers(display, 1);
	XStoreBuffer(display, data, strlen(data), 0);
return 0;
}

int BC_Window::clipboard_len()
{
	char *data2;
	int len;

	data2 = XFetchBuffer(display, &len, 0);
//printf("%ld\n", len);
	XFree(data2);
	return len;
return 0;
}

int BC_Window::from_clipboard(char *data, int max)
{
	char *data2;
	int len, i;

	data2 = XFetchBuffer(display, &len, 0);
	for(i = 0; i < len && i < max; i++)
		data[i] = data2[i];

	data[i] = 0;

	XFree(data2);
return 0;
}



// =================================== event handlers

int BC_Window::cycle_textboxes()
{
	int result = 0;
	BC_Tool *tool = 0;

	find_next_textbox(&tool, &result);

	if(result == 0)
	{
// no active textbox found
		return 0;
	}

	if(result == 1)
	{
// no next textbox found
		find_first_textbox(&tool);
	}

// activate the tool
	if(tool) tool->activate();
return 0;
}

int BC_Window::get_depth() { return depth; return 0;
}

int BC_Window::set_key_pressed(int new_value)
{ key_pressed = new_value; return 0;
}

int BC_Window::get_double_click()
{ return double_click; return 0;
}

int BC_Window::get_hidden()
{ return hidden; return 0;
}

int BC_Window::set_button_just_released()
{ button_just_released++; return 0;
}

int BC_Window::get_top_w()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return WidthOfScreen(screen_ptr);
return 0;
}

int BC_Window::get_top_h()
{
	Screen *screen_ptr = XDefaultScreenOfDisplay(display);
	return HeightOfScreen(screen_ptr);
return 0;
}

