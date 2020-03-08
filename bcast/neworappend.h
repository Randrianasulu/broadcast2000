#ifndef NEWORAPPEND_H
#define NEWORAPPEND_H

#include "bcbase.h"
#include "confirmsave.inc"
#include "file.inc"
#include "filehtal.inc"

class NewOrAppendNewButton;
class NewOrAppendCancelButton;
class NewOrAppendAppendButton;

class NewOrAppend
{
public:
	NewOrAppend();
	~NewOrAppend();
	
	int test_file(char *display, Asset *asset, FileHTAL *script = 0);
	// 2 append
	// 1 cancel
	// 0 replace
};

class NewOrAppendWindow : public BC_Window
{
public:
	NewOrAppendWindow(char *display, Asset *asset, int confidence);
	~NewOrAppendWindow();
	
	int confidence;
	int create_objects();
	Asset *asset;
	NewOrAppendCancelButton *cancel;
	NewOrAppendNewButton *ok;
	NewOrAppendAppendButton *append;
};

class NewOrAppendNewButton : public BC_BigButton
{
public:
	NewOrAppendNewButton(NewOrAppendWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	NewOrAppendWindow *nwindow;
};

class NewOrAppendAppendButton : public BC_BigButton
{
public:
	NewOrAppendAppendButton(NewOrAppendWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	NewOrAppendWindow *nwindow;
};

class NewOrAppendCancelButton : public BC_BigButton
{
public:
	NewOrAppendCancelButton(NewOrAppendWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	NewOrAppendWindow *nwindow;
};

#endif
