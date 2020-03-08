#ifndef RECCONFIRMDELETE_H
#define RECCONFIRMDELETE_H


#include "bcbase.h"

class RecConfirmDeleteOK;
class RecConfirmDeleteCancel;

class RecConfirmDelete : public BC_Window
{
public:
	RecConfirmDelete();
	~RecConfirmDelete();
	
	int create_objects(char *string);
	
	RecConfirmDeleteOK *ok;
	RecConfirmDeleteCancel *cancel;
};

class RecConfirmDeleteOK : public BC_BigButton
{
public:
	RecConfirmDeleteOK(RecConfirmDelete *window);
	~RecConfirmDeleteOK();
	
	int handle_event();
	int keypress_event();
	
	RecConfirmDelete *window;
};

class RecConfirmDeleteCancel : public BC_BigButton
{
public:
	RecConfirmDeleteCancel(RecConfirmDelete *window);
	~RecConfirmDeleteCancel();

	int handle_event();
	int keypress_event();
	
	RecConfirmDelete *window;
};



#endif
