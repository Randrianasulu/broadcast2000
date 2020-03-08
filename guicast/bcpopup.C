#include "bcpopup.h"




BC_Popup::BC_Popup(BC_WindowBase *parent_window, 
				int x,
				int y,
				int w, 
				int h, 
				int bg_color,
				int hide,
				BC_Pixmap *bg_pixmap)
 : BC_WindowBase()
{
	create_window(parent_window,
				"", 
				x,
				y,
				w, 
				h, 
				w, 
				h, 
				0,
				parent_window->top_level->private_color, 
				hide,
				bg_color,
				NULL,
				POPUP_WINDOW,
				bg_pixmap);
}


BC_Popup::~BC_Popup()
{
}

