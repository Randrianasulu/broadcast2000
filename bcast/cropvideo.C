#include <string.h>
#include "cropvideo.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "tracks.h"
#include "videowindow.h"
#include "videowindowgui.h"

CropVideo::CropVideo(MainWindow *mwindow)
 : BC_MenuItem("Crop Video..."), Thread()
{
	this->mwindow = mwindow;
}

CropVideo::~CropVideo()
{
}

int CropVideo::handle_event()
{
	start();
return 0;
}

void CropVideo::run()
{
	float aspect_w, aspect_h;
	int result = 0;
	{
		mwindow->video_window->start_cropping();
		CropVideoWindow window(mwindow, this);
		window.create_objects();
		result = window.run_window();
		mwindow->video_window->get_aspect_ratio(aspect_w, aspect_h);
		mwindow->video_window->stop_cropping();
	}

	if(!result)
	{
		int offsets[4], dummy_dimension[4];
		dummy_dimension[0] = dummy_dimension[1] = dummy_dimension[2] = dummy_dimension[3] = 0;
		offsets[0] = -(mwindow->video_window->gui->x1 + mwindow->video_window->gui->x2 - mwindow->output_w) / 2;
		offsets[1] = -(mwindow->video_window->gui->y1 + mwindow->video_window->gui->y2 - mwindow->output_h) / 2;
		offsets[2] = offsets[3] = 0;
		mwindow->undo->update_undo_edits("Crop", 0);
		
		mwindow->tracks->scale_video(dummy_dimension, offsets, 0);
		mwindow->track_w = mwindow->video_window->gui->x2 - mwindow->video_window->gui->x1;
		mwindow->track_h = mwindow->video_window->gui->y2 - mwindow->video_window->gui->y1;
		mwindow->output_w = mwindow->video_window->gui->x2 - mwindow->video_window->gui->x1;
		mwindow->output_h = mwindow->video_window->gui->y2 - mwindow->video_window->gui->y1;
		mwindow->aspect_w = aspect_w;
		mwindow->aspect_h = aspect_h;
		mwindow->video_window->resize_window();
		mwindow->draw();
		mwindow->undo->update_undo_edits();
		mwindow->changes_made = 1;
	}
	else
	{
	}
}

int CropVideo::load_defaults()
{
return 0;
}

int CropVideo::save_defaults()
{
return 0;
}

CropVideoWindow::CropVideoWindow(MainWindow *mwindow, CropVideo *thread)
 : BC_Window(ICONNAME ": Crop", 380, 75, 0, 0)
{
	this->mwindow = mwindow;
	this->thread = thread;
}

CropVideoWindow::~CropVideoWindow()
{
}

int CropVideoWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "Select a region to crop in the video output window"));
	y += 30;
	add_tool(new CropVideoOK(x, y, thread));
	x += 150;
	add_tool(new CropVideoCancel(x, y, thread));
return 0;
}




CropVideoOK::CropVideoOK(int x, int y, CropVideo *thread)
 : BC_BigButton(x, y, "OK")
{
	this->thread = thread;
}
CropVideoOK::~CropVideoOK()
{
}
int CropVideoOK::handle_event()
{
	set_done(0);
return 0;
}
int CropVideoOK::keypress_event()
{
	if(get_keypress() == 13) handle_event();
return 0;
}



CropVideoCancel::CropVideoCancel(int x, int y, CropVideo *thread)
 : BC_BigButton(x, y, "Cancel")
{
	this->thread = thread;
}
CropVideoCancel::~CropVideoCancel()
{
}
int CropVideoCancel::handle_event()
{
	set_done(1);
return 0;
}
int CropVideoCancel::keypress_event()
{
	if(get_keypress() == ESC) handle_event();
return 0;
}
