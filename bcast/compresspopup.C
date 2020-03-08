#include <string.h>
#include "compresspopup.h"
#include "file.h"
#include "quicktime.h"

CompressPopup::CompressPopup(int x, int y, int recording, char *text)
 : BC_PopupMenu(x, y, 180, compression_to_text(text))
{
	strcpy(format, text);
	this->recording = recording;
}

int CompressPopup::add_items()
{
	int item = 0;
	if(!recording) add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_DV)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_JPEG)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_MJPA)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_PNG)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(MOV_PNGA)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_RAW)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(MOV_RGBA)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_YUV420)));
	add_item(format_items[item++] = new CompressPopupItem(File::compressiontostr(QUICKTIME_YUV422)));
	compress_items = item;
	return 0;
}

CompressPopup::~CompressPopup()
{
	for(int i = 0; i < compress_items; i++) delete format_items[i];
}

char* CompressPopup::get_compression()
{
	File test_file;
	return test_file.strtocompression(get_text());
}

char* CompressPopup::compression_to_text(char *compression)
{
	File test_file;
	return test_file.compressiontostr(compression);
}

CompressPopupItem::CompressPopupItem(char *text)
 : BC_PopupItem(text)
{
}

CompressPopupItem::~CompressPopupItem()
{
}
	
int CompressPopupItem::handle_event()
{
	get_menu()->update(get_text());
	get_menu()->handle_event();
return 0;
}
