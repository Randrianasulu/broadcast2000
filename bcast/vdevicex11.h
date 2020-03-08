#ifndef VDEVICEX11_H
#define VDEVICEX11_H

#include "bcbase.h"
#include "mutex.h"
#include "thread.h"
#include "vdevicebase.h"

class VDeviceX11 : public VDeviceBase
{
public:
	VDeviceX11(VideoDevice *device);
	~VDeviceX11();

	int open_input();
	int close_all();
	int read_bitmap(VFrame *frame);
	int reset_parameters();
	ArrayList<int>* get_render_strategies();
	int get_output_w();     // get the output window width
	int get_output_h();     // get the output window height

	int open_output();
	int start_playback();
	int stop_playback();
	int output_visible();
// Get a bitmap
	BC_Bitmap* get_bitmap();
// After loading the bitmap with a picture, write it
	int write_buffer(VFrame *output);

private:
	BC_Bitmap *bitmap;
	int bitmap_w, bitmap_h;   // dimensions of buffers written to window
	ArrayList<int> render_strategies;
	VideoWindow *video_window; // window for output or input
	int color_model;
};

#endif
