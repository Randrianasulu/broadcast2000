#ifndef CONFIRMSAVE_H
#define CONFIRMSAVE_H

#include "bcbase.h"

class ConfirmSaveOkButton;
class ConfirmSaveCancelButton;

class ConfirmSave
{
public:
	ConfirmSave();
	~ConfirmSave();
	
	int test_file(const char *display, const char *path);    // return 1 if user cancels save
};

class ConfirmSaveWindow : public BC_Window
{
public:
	ConfirmSaveWindow(const char *display, char *filename);
	~ConfirmSaveWindow();
	
	int create_objects();
	char *filename;
	ConfirmSaveCancelButton *cancel;
	ConfirmSaveOkButton *ok;
};

class ConfirmSaveOkButton : public BC_BigButton
{
public:
	ConfirmSaveOkButton(ConfirmSaveWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	ConfirmSaveWindow *nwindow;
};

class ConfirmSaveCancelButton : public BC_BigButton
{
public:
	ConfirmSaveCancelButton(ConfirmSaveWindow *nwindow);
	
	int handle_event();
	int keypress_event();
	
	ConfirmSaveWindow *nwindow;
};

#endif
