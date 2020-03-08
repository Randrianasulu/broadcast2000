#include <string.h>
#include "assets.h"
#include "file.h"
#include "filesystem.h"
#include "fileformat.h"

FileFormat::FileFormat(char *display)
 : BC_Window(display, MEGREY, ICONNAME ": File Format", 375, 260, 415, 260)
{
}

FileFormat::~FileFormat()
{
	delete lohi;
	delete hilo;
	delete ok;
	delete cancel;
	delete signed_button;
	delete header_button;
	delete rate_button;
	delete channels_button;
	delete bits_button;
}

int FileFormat::create_objects(Asset *asset, char *string2)
{
// ================================= copy values
	this->asset = asset;
	create_objects_(string2);
return 0;
}

int FileFormat::create_objects_(char *string2)
{
	FileSystem dir;
	File file;
	char string[1024];
	add_tool(new BC_Title(5, 10, string2));
	add_tool(new BC_Title(5, 30, "Make sure the following values are correct:"));
	add_tool(ok = new FileFormatOkButton(this));
	add_tool(cancel = new FileFormatCancelButton(this));
	
	add_tool(new BC_Title(10, 62, "Channels:"));
	sprintf(string, "%d", asset->channels);
	add_tool(channels_button = new FileFormatChannels(this, string));
	add_tool(new BC_Title(180, 62, "Sample rate:"));
	sprintf(string, "%d", asset->rate);
	add_tool(rate_button = new FileFormatRate(this, string));
	add_tool(new BC_Title(10, 102, "Bits:"));
	add_tool(bits_button = new FileFormatBits(this, file.bitstostr(asset->bits)));
	add_tool(new BC_Title(180, 102, "Header length:"));
	sprintf(string, "%d", asset->header);
	add_tool(header_button = new FileFormatHeader(this, string));
	add_tool(new BC_Title(10, 145, "Byte order:"));
	add_tool(new BC_Title(10, 170, "Lo-Hi:", 1));
	add_tool(lohi = new FileFormatByteOrderLOHI(this, asset->byte_order));
	add_tool(new BC_Title(80, 170, "Hi-Lo:", 1));
	add_tool(hilo = new FileFormatByteOrderHILO(this, asset->byte_order ^ 1));
	add_tool(new BC_Title(180, 145, "Values are signed:"));
	add_tool(signed_button = new FileFormatSigned(this, asset->signed_));
return 0;
}

FileFormatOkButton::FileFormatOkButton(FileFormat *fwindow)
 : BC_BigButton(30, 215, "OK")
{
	this->fwindow = fwindow;
}

int FileFormatOkButton::handle_event()
{
	fwindow->set_done(0);
return 0;
}

int FileFormatOkButton::keypress_event()
{
	if(fwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

FileFormatCancelButton::FileFormatCancelButton(FileFormat *fwindow)
 : BC_BigButton(250, 215, "Cancel")
{
	this->fwindow = fwindow;
}

int FileFormatCancelButton::handle_event()
{
	fwindow->set_done(1);
return 0;
}

int FileFormatCancelButton::keypress_event()
{
	if(fwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

FileFormatChannels::FileFormatChannels(FileFormat *fwindow, char *text)
 : BC_TextBox(90, 60, 50, text)
{
	this->fwindow = fwindow;
}

int FileFormatChannels::handle_event()
{
	fwindow->asset->channels = atol(get_text());
return 0;
}

FileFormatRate::FileFormatRate(FileFormat *fwindow, char *text)
 : BC_TextBox(290, 60, 100, text)
{
	this->fwindow = fwindow;
}

int FileFormatRate::handle_event()
{
	fwindow->asset->rate = atol(get_text());
return 0;
}

FileFormatBits::FileFormatBits(FileFormat *fwindow, char *text)
 : BitsPopup(90, 100, fwindow->asset)
{
	this->fwindow = fwindow;
}

int FileFormatBits::handle_event()
{
	File file;
	fwindow->asset->bits = file.strtobits(get_text());
return 0;
}

FileFormatHeader::FileFormatHeader(FileFormat *fwindow, char *text)
 : BC_TextBox(290, 100, 100, text)
{
	this->fwindow = fwindow;
}

int FileFormatHeader::handle_event()
{
	fwindow->asset->header = atol(get_text());
return 0;
}

FileFormatByteOrderLOHI::FileFormatByteOrderLOHI(FileFormat *fwindow, int value)
 : BC_Radial(50, 170, 17, 17, value)
{
	this->fwindow = fwindow;
}

int FileFormatByteOrderLOHI::handle_event()
{
	fwindow->asset->byte_order = down;
	fwindow->hilo->update(down ^ 1);
return 0;
}

FileFormatByteOrderHILO::FileFormatByteOrderHILO(FileFormat *fwindow, int value)
 : BC_Radial(120, 170, 17, 17, value)
{
	this->fwindow = fwindow;
}

int FileFormatByteOrderHILO::handle_event()
{
	fwindow->asset->byte_order = down ^ 1;
	fwindow->lohi->update(down ^ 1);
return 0;
}

FileFormatSigned::FileFormatSigned(FileFormat *fwindow, int value)
 : BC_CheckBox(230, 170, 17, 17, value)
{
	this->fwindow = fwindow;
}

int FileFormatSigned::handle_event()
{
	fwindow->asset->signed_ = down;
return 0;
}
