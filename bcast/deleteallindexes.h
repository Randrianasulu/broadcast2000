#ifndef DELETEALLINDEXES_H
#define DELETEALLINDEXES_H

#include "bcbase.h"
#include "preferencesthread.inc"
#include "thread.h"

class DeleteAllIndexes : public BC_BigButton, public Thread
{
public:
	DeleteAllIndexes(int x, int y, PreferencesWindow *pwindow);
	~DeleteAllIndexes();
	
	void run();
	int handle_event();
	PreferencesWindow *pwindow;
};

class DeleteAllIndexesOK;
class DeleteAllIndexesCancel;

class ConfirmDeleteAllIndexes : public BC_Window
{
public:
	ConfirmDeleteAllIndexes(char *string);
	~ConfirmDeleteAllIndexes();
	
	int create_objects();
	DeleteAllIndexesOK *ok;
	DeleteAllIndexesCancel *cancel;
	char *string;
};

class DeleteAllIndexesOK : public BC_BigButton
{
public:
	DeleteAllIndexesOK(ConfirmDeleteAllIndexes *window);
	~DeleteAllIndexesOK();
	
	int handle_event();
	ConfirmDeleteAllIndexes *window;
};

class DeleteAllIndexesCancel : public BC_BigButton
{
public:
	DeleteAllIndexesCancel(ConfirmDeleteAllIndexes *window);
	~DeleteAllIndexesCancel();
	
	int handle_event();
	ConfirmDeleteAllIndexes *window;
};





#endif
