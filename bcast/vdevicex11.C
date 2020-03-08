#include <string.h>
#include "assets.h"
#include "mainwindow.h"
#include "preferences.h"
#include "strategies.inc"
#include "vdevicex11.h"
#include "vframe.h"
#include "videodevice.h"
#include "videowindow.h"
#include "videowindowgui.h"


VDeviceX11::VDeviceX11(VideoDevice *device)
 : VDeviceBase(device)
{
	reset_parameters();
	this->video_window = device->mwindow->video_window;
}

VDeviceX11::~VDeviceX11()
{
	close_all();
}

int VDeviceX11::reset_parameters()
{
	bitmap = 0;
	bitmap_w = 0;
	bitmap_h = 0;
	return 0;
return 0;
}

int VDeviceX11::open_input()
{
	return 0;
return 0;
}

int VDeviceX11::open_output()
{
// Get render strategies
	render_strategies.append(VRENDER_RGB888);
	render_strategies.append(VRENDER_VPIXEL);

	switch(video_window->gui->get_depth())
	{
		case 16:
			render_strategies.append(VRENDER_RGB565);
			break;

		case 32:
			render_strategies.append(VRENDER_BGR8880);
			break;
	}
	return 0;
return 0;
}

int VDeviceX11::close_all()
{
	if(bitmap)
		delete bitmap;
	reset_parameters();
	return 0;
return 0;
}

int VDeviceX11::read_bitmap(VFrame *frame)
{
	return 0;
return 0;
}

int VDeviceX11::start_playback()
{
// Record window is initialized when its monitor starts.
	video_window->init_video();
	return 0;
return 0;
}

int VDeviceX11::stop_playback()
{
	video_window->stop_video();
	if(bitmap) video_window->update(bitmap);
// Record window goes back to monitoring
// get the last frame played and store it in the video_out
	return 0;
return 0;
}

int VDeviceX11::write_buffer(VFrame *output)
{
	int i = 0;
// test size of bitmap against window size
	if(bitmap && 
		(bitmap_w != get_output_w() ||
		bitmap_h != get_output_h()))
	{
		delete bitmap;
		bitmap = 0;
	}

// allocate bitmap object
	if(!bitmap)
	{
		bitmap = device->get_bitmap();
	}

	bitmap_w = get_output_w();
	bitmap_h = get_output_h();

	bitmap->read_frame(output, 0, 0, device->out_w, device->out_h, device->mwindow->preferences->video_use_alpha);
// write to display now
	video_window->update(bitmap);
	return 0;
return 0;
}


int VDeviceX11::get_output_w()
{
	return video_window->get_w();
	return 0;
return 0;
}

int VDeviceX11::get_output_h()
{
	return video_window->get_h();
	return 0;
return 0;
}

ArrayList<int>* VDeviceX11::get_render_strategies()
{
	return &render_strategies;
}

int VDeviceX11::output_visible()
{
	if(!video_window->gui->get_hidden()) return 1; else return 0;
return 0;
}

BC_Bitmap* VDeviceX11::get_bitmap()
{
// test size of bitmap against window size
	if(bitmap && 
		(bitmap_w != get_output_w() ||
		bitmap_h != get_output_h()))
	{
		delete bitmap;
		bitmap = 0;
	}

// allocate bitmap object
	if(!bitmap)
	{
		bitmap = video_window->get_bitmap();
	}

	return bitmap;
}
