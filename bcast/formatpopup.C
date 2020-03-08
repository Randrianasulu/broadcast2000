#include <string.h>
#include "file.inc"
#include "formatpopup.h"
#include "pluginserver.h"


FormatPopup::FormatPopup(ArrayList<PluginServer*> *plugindb, int x, int y, char *text, int wr)
 : BC_PopupMenu(x, y, 175, text)
{
	strcpy(format, text);
	this->plugindb = plugindb;
	total_items = 0;
}

int FormatPopup::add_items()
{
	int i;
	add_item(format_items[total_items++] = new FormatPopupItem(this, "WAV"));
	add_item(format_items[total_items++] = new FormatPopupItem(this, "PCM"));
	add_item(format_items[total_items++] = new FormatPopupItem(this, MOV_NAME));
	add_item(format_items[total_items++] = new FormatPopupItem(this, JPEG_LIST_NAME));
// 	for(i = 0; i < plugindb->total; i++)
// 	{
// 		if(plugindb->values[i]->fileio)
// 		{
// 			add_item(format_items[total_items++] = new FormatPopupItem(this, plugindb->values[i]->title));
// 		}
// 	}
return 0;
}

FormatPopup::~FormatPopup()
{
	for(int i = 0; i < total_items; i++) delete format_items[i];
	total_items = 0;
}

int FormatPopup::handle_event()
{
	strcpy(format, get_text());
return 0;
}

FormatPopupItem::FormatPopupItem(FormatPopup *popup, char *text)
 : BC_PopupItem(text)
{
	this->popup = popup;
}

FormatPopupItem::~FormatPopupItem()
{
}
	
int FormatPopupItem::handle_event()
{
	popup->update(get_text());
	popup->handle_event();
return 0;
}
