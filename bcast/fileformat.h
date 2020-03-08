#ifndef FILEFORMAT_H
#define FILEFORMAT_H

#include "bcbase.h"

class FileFormatByteOrderLOHI;
class FileFormatByteOrderHILO;
class FileFormatSigned;
class FileFormatHeader;
class FileFormatRate;
class FileFormatChannels;
class FileFormatOkButton;
class FileFormatCancelButton;
class FileFormatBits;

#include "assets.inc"
#include "bitspopup.h"
#include "file.inc"

class FileFormat : public BC_Window
{
public:
	FileFormat(char *display = "");
	~FileFormat();

	int create_objects(Asset *asset, char *string2);

	int create_objects_(char *string2);

	Asset *asset; 

	FileFormatByteOrderLOHI *lohi;
	FileFormatByteOrderHILO *hilo;
	FileFormatSigned *signed_button;
	FileFormatHeader *header_button;
	FileFormatRate *rate_button;
	FileFormatBits *bits_button;
	FileFormatChannels *channels_button;
	FileFormatOkButton *ok;
	FileFormatCancelButton *cancel;
};

class FileFormatOkButton : public BC_BigButton
{
public:
	FileFormatOkButton(FileFormat *fwindow);
	
	int handle_event();
	int keypress_event();
	
	FileFormat *fwindow;
};

class FileFormatCancelButton : public BC_BigButton
{
public:
	FileFormatCancelButton(FileFormat *fwindow);
	
	int handle_event();
	int keypress_event();
	
	FileFormat *fwindow;
};

class FileFormatChannels : public BC_TextBox
{
public:
	FileFormatChannels(FileFormat *fwindow, char *text);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatRate : public BC_TextBox
{
public:
	FileFormatRate(FileFormat *fwindow, char *text);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatBits : public BitsPopup
{
public:
	FileFormatBits(FileFormat *fwindow, char *text);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatHeader : public BC_TextBox
{
public:
	FileFormatHeader(FileFormat *fwindow, char *text);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatByteOrderLOHI : public BC_Radial
{
public:
	FileFormatByteOrderLOHI(FileFormat *fwindow, int value);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatByteOrderHILO : public BC_Radial
{
public:
	FileFormatByteOrderHILO(FileFormat *fwindow, int value);
	
	int handle_event();
	
	FileFormat *fwindow;
};

class FileFormatSigned : public BC_CheckBox
{
public:
	FileFormatSigned(FileFormat *fwindow, int value);
	
	int handle_event();
	
	FileFormat *fwindow;
};

#endif

