#ifndef BITSPOPUP_H
#define BITSPOPUP_H

#include "assets.inc"
#include "bcbase.h"

class BitsPopupItem;

class BitsPopup : public BC_PopupMenu
{
public:
	BitsPopup(int x, int y, Asset *asset);
	~BitsPopup();
	
	virtual int handle_event();  // user does get_bits here
	int add_items();         // add initial items
	int get_bits();
	
	int bits;           // current setting 
	BitsPopupItem *bits_items[1024];
	int total_items;
	Asset *asset;
};

class BitsPopupItem : public BC_PopupItem
{
public:
	BitsPopupItem(char *text);
	~BitsPopupItem();
	
	int handle_event();
};





#endif
