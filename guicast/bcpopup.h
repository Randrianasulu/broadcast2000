#ifndef BCPOPUP_H
#define BCPOPUP_H

#include "arraylist.h"
#include "bcpixmap.inc"
#include "bcwindowbase.h"

class BC_Popup : public BC_WindowBase
{
public:
	BC_Popup(BC_WindowBase *parent_window, 
				int x, 
				int y, 
				int w, 
				int h, 
				int bg_color, 
				int hide = 0, 
				BC_Pixmap *bg_pixmap = 0);
	virtual ~BC_Popup();

	int initialize() { return 0; };
private:
};


#endif
