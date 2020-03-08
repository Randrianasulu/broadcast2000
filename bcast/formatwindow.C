#include <string.h>
#include "assets.h"
#include "file.h"
#include "formatwindow.h"


FormatAWindow::FormatAWindow(Asset *asset, int *dither)
 : BC_Window(ICONNAME ": File format", 410, 
 	(asset->format == WAV || asset->format == MOV) ? 115 : 185, 
	0, 0)
{ this->asset = asset; this->dither = dither; }

FormatAWindow::~FormatAWindow()
{
}

int FormatAWindow::create_objects()
{
	int x;
	int init_x;
	int y = 10;
	File file;
	x = init_x = 10;

	add_tool(new BC_Title(x, y, "Set parameters for this audio format:"));
	y += 30;
	add_tool(new BC_Title(x, y, "Bits:"));
	x += 45;
	add_tool(new FormatBits(x, y, asset));
	x += 100;
	add_tool(new FormatDither(x, y, this->dither));

	if(asset->format == PCM || asset->format == MOV)
	{
		x += 90;
		add_tool(new FormatSigned(x, y, asset));
	}
	y += 40;
	x = init_x;

	if(asset->format == PCM)
	{
		add_tool(new BC_Title(x, y, "Byte order:"));
		y += 25;
		add_tool(new BC_Title(x, y, "HiLo:", SMALLFONT));
		add_tool(hilo_button = new FormatHILO(x + 30, y, asset));
		x += 50;
		add_tool(new BC_Title(x, y, "LoHi:", SMALLFONT));
		add_tool(lohi_button = new FormatLOHI(x + 30, y, hilo_button, asset));
		hilo_button->lohi = lohi_button;
		y += 30;
	}

	x = init_x;

	add_tool(new FormatOK(x + 170, y, this));
return 0;
}


int FormatAWindow::close_event()
{
	set_done(0);
return 0;
}




FormatVWindow::FormatVWindow(Asset *asset, int recording)
 : BC_Window(ICONNAME ": File format", 510, 115, 0, 0)
{ 
	this->asset = asset; 
	this->recording = recording; 
}

FormatVWindow::~FormatVWindow()
{
}

int FormatVWindow::create_objects()
{
	int x, y = 10;
	int init_x;

	init_x = x = 10;

	if(asset->format == MOV)
	{
		add_tool(new BC_Title(x, y, "Set parameters for this video format:"));
		y += 30;
		add_tool(new BC_Title(x, y, "Compression:"));
		x += 110;
		add_tool(new FormatCompress(x, y, recording, asset, asset->compression));
		x += 190;
		add_tool(new BC_Title(x, y, "Quality:"));
		x += 70;
		add_tool(new FormatQuality(x, y, asset, asset->quality));
		y += 40;
		x = init_x;
	}
	else
	if(asset->format == JPEG_LIST)
	{
		add_tool(new BC_Title(x, y, "Set parameters for this video format:"));
		y += 30;
		add_tool(new BC_Title(x, y, "Quality:"));
		x += 70;
		add_tool(new FormatQuality(x, y, asset, asset->quality));
		y += 40;
		x = init_x;
	}
	else
	{
		add_tool(new BC_Title(x, y, "Video is not supported in this format."));
		y += 40;
	}

	add_tool(new FormatOK(x + 170, y, this));
return 0;
}

int FormatVWindow::close_event()
{
	set_done(0);
return 0;
}







FormatCompress::FormatCompress(int x, int y, int recording, Asset *asset, char* default_)
 : CompressPopup(x, y, recording, default_)
{ 
	this->asset = asset; 
}
FormatCompress::~FormatCompress() {}
int FormatCompress::handle_event()
{
	strcpy(asset->compression, get_compression());
return 0;
}

FormatQuality::FormatQuality(int x, int y, Asset *asset, int default_)
 : BC_ISlider(x, y, 100, 30, 100, default_, 0, 100, LTGREY, MEGREY, 1)
{ this->asset = asset; }
FormatQuality::~FormatQuality() {}
int FormatQuality::handle_event()
{
	asset->quality = get_value();
return 0;
}



FormatBits::FormatBits(int x, int y, Asset *asset)
 : BitsPopup(x, y, asset)
{ this->asset = asset; }
FormatBits::~FormatBits() {}
int FormatBits::handle_event()
{
	asset->bits = get_bits();
return 0;
}



FormatDither::FormatDither(int x, int y, int *dither)
 : BC_CheckBox(x, y, 17, 17, *dither, "Dither")
{ this->dither = dither; }
FormatDither::~FormatDither() {}
int FormatDither::handle_event()
{
	*dither = get_value();
return 0;
}




FormatSigned::FormatSigned(int x, int y, Asset *asset)
 : BC_CheckBox(x, y, 17, 17, asset->signed_, "Signed")
{ 
	this->asset = asset; 
}
FormatSigned::~FormatSigned() {}
int FormatSigned::handle_event()
{
	asset->signed_ = get_value();
return 0;
}






FormatHILO::FormatHILO(int x, int y, Asset *asset)
 : BC_Radial(x, y, 17, 17, asset->byte_order ^ 1)
{
	this->asset = asset;
}
FormatHILO::~FormatHILO() {}

int FormatHILO::handle_event()
{
	asset->byte_order = get_value() ^ 1;
	lohi->update(get_value() ^ 1);
return 0;
}

FormatLOHI::FormatLOHI(int x, int y, FormatHILO *hilo, Asset *asset)
 : BC_Radial(x, y, 17, 17, asset->byte_order)
{
	this->hilo = hilo;
	this->asset = asset;
}
FormatLOHI::~FormatLOHI() {}

int FormatLOHI::handle_event()
{
	asset->byte_order = get_value();
	hilo->update(get_value() ^ 1);
return 0;
}


FormatOK::FormatOK(int x, int y, BC_Window *window)
 : BC_BigButton(x, y, "OK")
{
	this->window = window;
}

FormatOK::~FormatOK()
{
}

int FormatOK::handle_event()
{
	window->set_done(0);
return 0;
}

int FormatOK::keypress_event()
{
	if(get_keypress() == 13) window->set_done(0);
return 0;
}
