#ifndef VDEVICEBASE_H
#define VDEVICEBASE_H

#include "assets.inc"
#include "bcbase.h"
#include "videodevice.inc"

class VDeviceBase
{
public:
	VDeviceBase(VideoDevice *device);
	virtual ~VDeviceBase();

	virtual int open_input() { return 1; };
	virtual int close_all() { return 1; };
	virtual int read_buffer(VFrame *frame) { return 1; };
	virtual int write_buffer(VFrame *output) { return 1; };
	virtual ArrayList<int>* get_render_strategies() { return 0; };
	virtual int get_shared_data(unsigned char *data, long size) { return 0; };
	virtual int stop_sharing() { return 0; };
	virtual int interrupt_crash() { return 0; };

	virtual int open_output() { return 1; };
	virtual int output_visible() { return 0; };
	virtual int start_playback() { return 1; };
	virtual int stop_playback() { return 1; };
	virtual BC_Bitmap* get_bitmap() { return 0; };

	VideoDevice *device;
};

#endif
