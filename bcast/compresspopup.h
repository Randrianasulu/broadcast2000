#ifndef COMPRESSPOPUP_H
#define COMPRESSPOPUP_H


#include "bcbase.h"
#include "compresspopup.inc"

class CompressPopup : public BC_PopupMenu
{
public:
	CompressPopup(int x, int y, int recording, char *text);
	~CompressPopup();
	
	virtual int handle_event() { return 0; };
	char *get_compression();
	char *compression_to_text(char *compression);
	
	int add_items();         // add initial items
	char format[256];           // current setting 
	CompressPopupItem *format_items[COMPRESS_ITEMS];
	int recording;
	int compress_items;
};

class CompressPopupItem : public BC_PopupItem
{
public:
	CompressPopupItem(char *text);
	~CompressPopupItem();
	
	int handle_event();
};







#endif
