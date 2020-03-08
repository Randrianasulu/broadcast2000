#include "bcipc.h"
#include "bclistbox.inc"
#include "bcresources.h"
#include "bcsignals.h"
#include "bcwindowbase.h"
#include "colors.h"
#include "colormodels.h"

#include "images/folder_png.h"
#include "images/heroine_file_png.h"
#include "images/list_bg_png.h"
#include "images/listbox_button_dn_png.h"
#include "images/listbox_button_hi_png.h"
#include "images/listbox_button_up_png.h"
#include "images/menu_bg_png.h"
#include "images/window_bg_png.h"
#include "vframe.h"

#include <locale.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

int BC_Resources::error = 0;

VFrame* BC_Resources::bg_image = new VFrame(window_bg_png);
VFrame* BC_Resources::menu_bg = new VFrame(menu_bg_png);

#include "images/file_film_png.h"
#include "images/file_folder_png.h"
#include "images/file_sound_png.h"
#include "images/file_unknown_png.h"
VFrame* BC_Resources::type_to_icon[] = 
{
	new VFrame(file_folder_png),
	new VFrame(file_unknown_png),
	new VFrame(file_film_png),
	new VFrame(file_sound_png)
};

char* BC_Resources::small_font = "-*-helvetica-medium-r-normal-*-10-*";
char* BC_Resources::medium_font = "-*-helvetica-bold-r-normal-*-14-*";
char* BC_Resources::large_font = "-*-helvetica-bold-r-normal-*-18-*";
char* BC_Resources::small_fontset = "6x12,*";
char* BC_Resources::medium_fontset = "7x14,*";
char* BC_Resources::large_fontset = "8x16,*";

suffix_to_type_t BC_Resources::suffix_to_type[] = 
{
	{ "m2v", ICON_FILM },
	{ "mov", ICON_FILM },
	{ "mp2", ICON_SOUND },
	{ "mp3", ICON_SOUND },
	{ "mpg", ICON_FILM },
	{ "vob", ICON_FILM },
	{ "wav", ICON_SOUND }
};

BC_Signals* BC_Resources::signal_handler = 0;

int BC_Resources::x_error_handler(Display *display, XErrorEvent *event)
{
	char string[1024];
	XGetErrorText(event->display, event->error_code, string, 1024);
//	printf("BC_Resources::x_error_handler: %s\n", string);
	BC_Resources::error = 1;
	return 0;
}

BC_Resources::BC_Resources()
{
	use_xvideo = 1;
#include "images/cancel_up_png.h"
#include "images/cancel_uphi_png.h"
#include "images/cancel_downhi_png.h"
	static VFrame* default_cancel_images[] = 
	{
		new VFrame(cancel_up_png),
		new VFrame(cancel_uphi_png),
		new VFrame(cancel_downhi_png)
	};

#include "images/ok_up_png.h"
#include "images/ok_uphi_png.h"
#include "images/ok_downhi_png.h"
	static VFrame* default_ok_images[] = 
	{
		new VFrame(ok_up_png),
		new VFrame(ok_uphi_png),
		new VFrame(ok_downhi_png)
	};

#include "images/usethis_up_png.h"
#include "images/usethis_uphi_png.h"
#include "images/usethis_dn_png.h"
	static VFrame* default_usethis_images[] = 
	{
		new VFrame(usethis_up_png),
		new VFrame(usethis_uphi_png),
		new VFrame(usethis_dn_png)
	};

#include "images/checkbox_checked_png.h"
#include "images/checkbox_down_png.h"
#include "images/checkbox_checkedhi_png.h"
#include "images/checkbox_up_png.h"
#include "images/checkbox_uphi_png.h"
	static VFrame* default_checkbox_images[] =  
	{
		new VFrame(checkbox_up_png),
		new VFrame(checkbox_uphi_png),
		new VFrame(checkbox_checked_png),
		new VFrame(checkbox_down_png),
		new VFrame(checkbox_checkedhi_png)
	};

#include "images/radial_checked_png.h"
#include "images/radial_down_png.h"
#include "images/radial_checkedhi_png.h"
#include "images/radial_up_png.h"
#include "images/radial_uphi_png.h"
	static VFrame* default_radial_images[] =  
	{
		new VFrame(radial_up_png),
		new VFrame(radial_uphi_png),
		new VFrame(radial_checked_png),
		new VFrame(radial_down_png),
		new VFrame(radial_checkedhi_png)
	};

	static VFrame* default_label_images[] =  
	{
		new VFrame(radial_up_png),
		new VFrame(radial_uphi_png),
		new VFrame(radial_checked_png),
		new VFrame(radial_down_png),
		new VFrame(radial_checkedhi_png)
	};


#include "images/file_text_up_png.h"
#include "images/file_text_uphi_png.h"
#include "images/file_text_dn_png.h"
#include "images/file_icons_up_png.h"
#include "images/file_icons_uphi_png.h"
#include "images/file_icons_dn_png.h"
#include "images/file_newfolder_up_png.h"
#include "images/file_newfolder_uphi_png.h"
#include "images/file_newfolder_dn_png.h"
#include "images/file_updir_up_png.h"
#include "images/file_updir_uphi_png.h"
#include "images/file_updir_dn_png.h"
	static VFrame* default_filebox_text_images[] = 
	{
		new VFrame(file_text_up_png),
		new VFrame(file_text_uphi_png),
		new VFrame(file_text_dn_png)
	};

	static VFrame* default_filebox_icons_images[] = 
	{
		new VFrame(file_icons_up_png),
		new VFrame(file_icons_uphi_png),
		new VFrame(file_icons_dn_png)
	};

	static VFrame* default_filebox_updir_images[] =  
	{
		new VFrame(file_updir_up_png),
		new VFrame(file_updir_uphi_png),
		new VFrame(file_updir_dn_png)
	};

	static VFrame* default_filebox_newfolder_images[] = 
	{
		new VFrame(file_newfolder_up_png),
		new VFrame(file_newfolder_uphi_png),
		new VFrame(file_newfolder_dn_png)
	};

	static VFrame* default_listbox_button[] = 
	{
		new VFrame(listbox_button_up_png),
		new VFrame(listbox_button_hi_png),
		new VFrame(listbox_button_dn_png)
	};

	static VFrame* default_listbox_bg = new VFrame(list_bg_png);

#include "images/slider_left_png.h"
#include "images/slider_mid_png.h"
#include "images/slider_right_png.h"
#include "images/slider_dn_png.h"
#include "images/slider_up_png.h"
#include "images/slider_uphi_png.h"
	static VFrame* default_horizontal_slider[] = 
	{
		new VFrame(slider_up_png),
		new VFrame(slider_uphi_png),
		new VFrame(slider_dn_png),
		new VFrame(slider_left_png),
		new VFrame(slider_mid_png),
		new VFrame(slider_right_png)
	};

#include "images/fadevertical_left_png.h"
#include "images/fadevertical_mid_png.h"
#include "images/fadevertical_right_png.h"
#include "images/fadevertical_up_png.h"
#include "images/fadevertical_uphi_png.h"
	static VFrame* default_vertical_slider[] = 
	{
		new VFrame(fadevertical_up_png),
		new VFrame(fadevertical_uphi_png),
		new VFrame(fadevertical_left_png),
		new VFrame(fadevertical_mid_png),
		new VFrame(fadevertical_right_png)
	};

#include "images/pot_hi_png.h"
#include "images/pot_up_png.h"
#include "images/pot_dn_png.h"
	static VFrame *default_pot_images[] = 
	{
		new VFrame(pot_up_png),
		new VFrame(pot_hi_png),
		new VFrame(pot_dn_png)
	};

#include "images/progress_hi_left_png.h"
#include "images/progress_hi_mid_png.h"
#include "images/progress_hi_right_png.h"
#include "images/progress_left_png.h"
#include "images/progress_mid_png.h"
#include "images/progress_right_png.h"
	static VFrame* default_progress_images[] = 
	{
		new VFrame(progress_left_png),
		new VFrame(progress_mid_png),
		new VFrame(progress_right_png),
		new VFrame(progress_hi_left_png),
		new VFrame(progress_hi_mid_png),
		new VFrame(progress_hi_right_png)
	};


#include "images/pan_bg_png.h"
#include "images/pan_bg_hi_png.h"
#include "images/pan_channel_png.h"
#include "images/pan_stick_png.h"
	static VFrame* default_pan_bg = new VFrame(pan_bg_png);
	static VFrame* default_pan_bg_hi = new VFrame(pan_bg_hi_png);
	static VFrame* default_pan_channel = new VFrame(pan_channel_png);
	static VFrame* default_pan_stick = new VFrame(pan_stick_png);

#include "images/7seg_small/0_png.h"
#include "images/7seg_small/1_png.h"
#include "images/7seg_small/2_png.h"
#include "images/7seg_small/3_png.h"
#include "images/7seg_small/4_png.h"
#include "images/7seg_small/5_png.h"
#include "images/7seg_small/6_png.h"
#include "images/7seg_small/7_png.h"
#include "images/7seg_small/8_png.h"
#include "images/7seg_small/9_png.h"
#include "images/7seg_small/colon_png.h"
#include "images/7seg_small/space_png.h"
	static VFrame* default_medium_7segment[] = 
	{
		new VFrame(_0_png),
		new VFrame(_1_png),
		new VFrame(_2_png),
		new VFrame(_3_png),
		new VFrame(_4_png),
		new VFrame(_5_png),
		new VFrame(_6_png),
		new VFrame(_7_png),
		new VFrame(_8_png),
		new VFrame(_9_png),
		new VFrame(colon_png),
		new VFrame(space_png)
	};

#include "images/tumblerbottom_dn_png.h"
#include "images/tumblertop_dn_png.h"
#include "images/tumbler_hi_png.h"
#include "images/tumbler_up_png.h"
	static VFrame* default_tumbler_data[] = 
	{
		new VFrame(tumbler_up_png),
		new VFrame(tumbler_hi_png),
		new VFrame(tumblerbottom_dn_png),
		new VFrame(tumblertop_dn_png)
	};

#include "images/xmeter_normal_png.h"
#include "images/xmeter_green_png.h"
#include "images/xmeter_red_png.h"
#include "images/xmeter_yellow_png.h"
#include "images/over_horiz_png.h"
#include "images/ymeter_normal_png.h"
#include "images/ymeter_green_png.h"
#include "images/ymeter_red_png.h"
#include "images/ymeter_yellow_png.h"
#include "images/over_vertical_png.h"
	static VFrame* default_xmeter_data[] =
	{
		new VFrame(xmeter_normal_png),
		new VFrame(xmeter_green_png),
		new VFrame(xmeter_red_png),
		new VFrame(xmeter_yellow_png),
		new VFrame(over_horiz_png)
	};

	static VFrame* default_ymeter_data[] =
	{
		new VFrame(ymeter_normal_png),
		new VFrame(ymeter_green_png),
		new VFrame(ymeter_red_png),
		new VFrame(ymeter_yellow_png),
		new VFrame(over_vertical_png)
	};

#include "images/generic_button_leftdn_png.h"
#include "images/generic_button_lefthi_png.h"
#include "images/generic_button_leftup_png.h"
#include "images/generic_button_middn_png.h"
#include "images/generic_button_midhi_png.h"
#include "images/generic_button_midup_png.h"
#include "images/generic_button_rightdn_png.h"
#include "images/generic_button_righthi_png.h"
#include "images/generic_button_rightup_png.h"
	
	static VFrame* default_generic_button_data[] = 
	{
		new VFrame(generic_button_leftdn_png),
		new VFrame(generic_button_lefthi_png),
		new VFrame(generic_button_leftup_png),
		new VFrame(generic_button_middn_png),
		new VFrame(generic_button_midhi_png),
		new VFrame(generic_button_midup_png),
		new VFrame(generic_button_rightdn_png),
		new VFrame(generic_button_righthi_png),
		new VFrame(generic_button_rightup_png),
	};
	
	generic_button_images = default_generic_button_data;

	use_shm = -1;

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

	tumble_data = default_tumbler_data;
	tumble_duration = 150;

	ok_images = default_ok_images;
	cancel_images = default_cancel_images;
	usethis_button_images = default_usethis_images;

	filebox_text_images = default_filebox_text_images;
	filebox_icons_images = default_filebox_icons_images;
	filebox_updir_images = default_filebox_updir_images;
	filebox_newfolder_images = default_filebox_newfolder_images;

	checkbox_images = default_checkbox_images;
	radial_images = default_radial_images;
	label_images = default_label_images;
	horizontal_slider = default_horizontal_slider;

	menu_light = LTCYAN;
	menu_highlighted = LTBLUE;
	menu_down = MDCYAN;
	menu_up = MECYAN;
	menu_shadow = DKCYAN;

	text_default = BLACK;
	text_background = WHITE;
	highlight_inverse = WHITE ^ BLUE;
	text_highlight = BLUE;

// Delays must all be different for repeaters
	double_click = 300;
	blink_rate = 250;
	scroll_repeat = 150;
	tooltip_delay = 1000;
	tooltip_bg_color = YELLOW;
	tooltips_enabled = 1;
	
	filebox_mode = LISTBOX_TEXT;
	sprintf(filebox_filter, "*");
	filebox_w = 640;
	filebox_h = 480;
	
	listbox_button = default_listbox_button;
	listbox_bg = default_listbox_bg;

	pot_images = default_pot_images;
	pot_x1 = pot_images[0]->get_w() / 2 - 2;
	pot_y1 = pot_images[0]->get_h() / 2 - 2;;
	pot_r = pot_x1;

	pan_bg = default_pan_bg;
	pan_bg_hi = default_pan_bg_hi;
	pan_channel = default_pan_channel;
	pan_stick = default_pan_stick;

	progress_images = default_progress_images;

	xmeter_images = default_xmeter_data;
	ymeter_images = default_ymeter_data;
	medium_7segment = default_medium_7segment;
	use_fontset = 0;
	if(use_fontset) setlocale(LC_ALL, "");
	drag_radius = 10;
	recursive_resizing = 1;
}

BC_Resources::~BC_Resources()
{
}

int BC_Resources::initialize_display(BC_WindowBase *window)
{
// Set up IPC cleanup handlers
	bc_init_ipc();

// Test for shm.  Must come before yuv test
	init_shm(window);
	return 0;
}

int BC_Resources::init_shm(BC_WindowBase *window)
{
	use_shm = 1;
	XSetErrorHandler(BC_Resources::x_error_handler);

	if(!XShmQueryExtension(window->display)) use_shm = 0;
	else
	{
		XShmSegmentInfo test_shm;
		XImage *test_image;
		unsigned char *data;
		test_image = XShmCreateImage(window->display, window->vis, window->default_depth,
                ZPixmap, (char*)NULL, &test_shm, 5, 5);

		test_shm.shmid = shmget(IPC_PRIVATE, 5 * test_image->bytes_per_line, (IPC_CREAT | 0777 ));
		data = (unsigned char *)shmat(test_shm.shmid, NULL, 0);
    	shmctl(test_shm.shmid, IPC_RMID, 0);
		BC_Resources::error = 0;
 	   	XShmAttach(window->display, &test_shm);
    	XSync(window->display, False);
		if(BC_Resources::error) use_shm = 0;
		XDestroyImage(test_image);
		shmdt(test_shm.shmaddr);
	}
	return 0;
}

int BC_Resources::get_bg_color() { return bg_color; }

int BC_Resources::get_bg_shadow1() { return bg_shadow1; }

int BC_Resources::get_bg_shadow2() { return bg_shadow2; }

int BC_Resources::get_bg_light1() { return bg_light1; }

int BC_Resources::get_bg_light2() { return bg_light2; }

