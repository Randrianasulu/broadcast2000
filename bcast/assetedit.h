#ifndef ASSETEDIT_H
#define ASSETEDIT_H

#include "assetmanager.inc"
#include "assets.inc"
#include "bcbase.h"
#include "bitspopup.h"
#include "browsebutton.h"
#include "formatpopup.h"
#include "mainwindow.h"
#include "thread.h"

class AssetEditByteOrderHILO;
class AssetEditByteOrderLOHI;
class AssetEditPath;
class AssetEditPathText;

class AssetEdit : public Thread
{
public:
	AssetEdit(AssetManagerThread *thread);
	~AssetEdit();
	
	int set_asset(Asset *asset);
	void run();

	Asset *asset;
	AssetManagerThread *thread;
};



// Pcm is the only format users should be able to fix.
// All other formats display information about the file in read-only.

class AssetEditWindow : public BC_Window
{
public:
	AssetEditWindow(MainWindow *mwindow, Asset *asset);
	~AssetEditWindow();

	int create_objects();
	Asset *asset;
	AssetEditPathText *path_text;
	AssetEditPath *path_button;
	AssetEditByteOrderHILO *hilo;
	AssetEditByteOrderLOHI *lohi;
	int allow_edits;
	MainWindow *mwindow;
};


class AssetEditPath : public BrowseButton
{
public:
	AssetEditPath(AssetEditWindow *fwindow, int y, BC_TextBox *textbox, const char *text, const char *window_title = "2000: Path", const char *window_caption = "Select a file");
	~AssetEditPath();
	
	AssetEditWindow *fwindow;
};


class AssetEditPathText : public BC_TextBox
{
public:
	AssetEditPathText(AssetEditWindow *fwindow, int y);
	~AssetEditPathText();
	int handle_event();

	AssetEditWindow *fwindow;
};



class AssetEditFormat : public FormatPopup
{
public:
	AssetEditFormat(AssetEditWindow *fwindow, const char* default_, int y);
	~AssetEditFormat();
	
	int handle_event();
	AssetEditWindow *fwindow;
};


class AssetEditOkButton : public BC_BigButton
{
public:
	AssetEditOkButton(AssetEditWindow *fwindow, int y);
	
	int handle_event();
	int keypress_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditCancelButton : public BC_BigButton
{
public:
	AssetEditCancelButton(AssetEditWindow *fwindow, int y);
	
	int handle_event();
	int keypress_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditChannels : public BC_TextBox
{
public:
	AssetEditChannels(AssetEditWindow *fwindow, const char *text, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditRate : public BC_TextBox
{
public:
	AssetEditRate(AssetEditWindow *fwindow, const char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditFRate : public BC_TextBox
{
public:
	AssetEditFRate(AssetEditWindow *fwindow, const char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditBits : public BitsPopup
{
public:
	AssetEditBits(AssetEditWindow *fwindow, const char *text, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditHeader : public BC_TextBox
{
public:
	AssetEditHeader(AssetEditWindow *fwindow, const char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditByteOrderLOHI : public BC_Radial
{
public:
	AssetEditByteOrderLOHI(AssetEditWindow *fwindow, int value, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditByteOrderHILO : public BC_Radial
{
public:
	AssetEditByteOrderHILO(AssetEditWindow *fwindow, int value, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditSigned : public BC_CheckBox
{
public:
	AssetEditSigned(AssetEditWindow *fwindow, int value, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

#endif
