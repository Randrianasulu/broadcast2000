#ifndef FORMATPOPUP_H
#define FORMATPOPUP_H



#include "bcbase.h"
#include "formatpopup.inc"
#include "pluginserver.inc"

class FormatPopup : public BC_PopupMenu
{
public:
// set wr to 1 for writable file formats
	FormatPopup(ArrayList<PluginServer*> *plugindb, int x, int y, char *text, int wr = 1);
	~FormatPopup();

	virtual int handle_event();  // user copies text to value here
	int add_items();         // add initial items
	char format[1024];           // current setting 
	FormatPopupItem *format_items[1024];
	int total_items;
	ArrayList<PluginServer*> *plugindb;
};

class FormatPopupItem : public BC_PopupItem
{
public:
	FormatPopupItem(FormatPopup *popup, char *text);
	~FormatPopupItem();

	int handle_event();
	FormatPopup *popup;
};







#endif
