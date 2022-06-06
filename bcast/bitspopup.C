#include <string.h>
#include "assets.h"
#include "bitspopup.h"
#include "file.h"


BitsPopup::BitsPopup(int x, int y, Asset *asset)
 : BC_PopupMenu(x, y, 70, "")
{
	this->asset = asset;
	bits = asset->bits;
	total_items = 0;
}

int BitsPopup::add_items()
{
	File file;
	add_item(bits_items[0] = new BitsPopupItem(file.bitstostr(BITSLINEAR8)));
	add_item(bits_items[1] = new BitsPopupItem(file.bitstostr(BITSLINEAR16)));
	add_item(bits_items[2] = new BitsPopupItem(file.bitstostr(BITSLINEAR24)));
	total_items = 3;
	
	if(asset->format == PCM || asset->format == MOV)
	{
		add_item(bits_items[total_items] = new BitsPopupItem(file.bitstostr(BITSULAW)));
		total_items++;
	}

	if(asset->format == MOV)
	{
		add_item(bits_items[total_items++] = new BitsPopupItem(file.bitstostr(BITSIMA4)));
		//add_item(bits_items[total_items++] = new BitsPopupItem(file.bitstostr(BITSWMX2)));
	}

	update(file.bitstostr(bits));
return 0;
}

BitsPopup::~BitsPopup()
{
	for(int i = 0; i < total_items; i++) delete bits_items[i];
	total_items = 0;
}
	
int BitsPopup::handle_event()
{
return 0;
}

int BitsPopup::get_bits()
{
	File file;
	return file.strtobits(get_text());
return 0;
}

BitsPopupItem::BitsPopupItem(const char *text)
 : BC_PopupItem(text)
{
}

BitsPopupItem::~BitsPopupItem()
{
}
	
int BitsPopupItem::handle_event()
{
	get_menu()->update(get_text());
	get_menu()->handle_event();
	return 1;
return 0;
}
