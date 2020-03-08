#ifndef CROPVIDEO_H
#define CROPVIDEO_H

#include "bcbase.h"
#include "mainwindow.inc"
#include "thread.h"
#include "cropvideo.inc"

class CropVideo : public BC_MenuItem, public Thread
{
public:
	CropVideo(MainWindow *mwindow);
	~CropVideo();

	int handle_event();
	void run();
	int load_defaults();
	int save_defaults();
	int fix_aspect_ratio();

	MainWindow *mwindow;
};

class CropVideoWindow : public BC_Window
{
public:
	CropVideoWindow(MainWindow *mwindow, CropVideo *thread);
	~CropVideoWindow();

	int create_objects();

	CropVideo *thread;
	MainWindow *mwindow;
};

class CropVideoOK : public BC_BigButton
{
public:
	CropVideoOK(int x, int y, CropVideo *thread);
	~CropVideoOK();

	int handle_event();
	int keypress_event();
	CropVideo *thread;
};


class CropVideoCancel : public BC_BigButton
{
public:
	CropVideoCancel(int x, int y, CropVideo *thread);
	~CropVideoCancel();

	int handle_event();
	int keypress_event();
	CropVideo *thread;
};


#endif
