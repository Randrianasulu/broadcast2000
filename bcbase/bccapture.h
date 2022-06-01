#ifndef BCCAPTURE_H
#define BCCAPTURE_H

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#include "sizes.h"
#include "vframe.inc"

class BC_ImportPixel;

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} BC_CapturePixel;


class BC_Capture
{
public:
	BC_Capture(int w, int h, const char *display_text = "");
	virtual ~BC_Capture();

	int init_window(const char *display_text);
	int capture_frame(VFrame *frame, int x1, int y1);
	int get_w();
	int get_h();

	int w, h, depth;
	unsigned char **row_data;

private:
	int allocate_data();
	int delete_data();
	int get_top_w();
	int get_top_h();

	int use_shm;
	unsigned char *data;
	XImage *ximage;
	XShmSegmentInfo shm_info;
	Display* display;
	Window rootwin;
	Visual *vis;
	int bits_per_pixel;
	int screen;
	long shm_event_type;
	int client_byte_order, server_byte_order;
	BC_ImportPixel *import_pixel;
};

class BC_ImportPixelBase;
class BC_ExportPixelBase;

class BC_ImportPixel
{
public:
	BC_ImportPixel(int depth, int byte_swap);
	~BC_ImportPixel();

	int import_pixel(BC_CapturePixel &output, unsigned char **input);

	BC_ImportPixelBase *importer;
};

class BC_ImportPixelBase
{
public:
	BC_ImportPixelBase();
	virtual ~BC_ImportPixelBase();

	virtual int import_pixel(BC_CapturePixel &output, unsigned char **input) { return 0; };
};

class BC_ImportPixel8 : public BC_ImportPixelBase
{
public:
	BC_ImportPixel8();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

class BC_ImportPixel16 : public BC_ImportPixelBase
{
public:
	BC_ImportPixel16();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
	unsigned TWO temp;
};

class BC_ImportPixel24 : public BC_ImportPixelBase
{
public:
	BC_ImportPixel24();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

class BC_ImportPixel32 : public BC_ImportPixelBase
{
public:
	BC_ImportPixel32();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

class BC_ImportPixel8Swap : public BC_ImportPixelBase
{
public:
	BC_ImportPixel8Swap();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

class BC_ImportPixel16Swap : public BC_ImportPixelBase
{
public:
	BC_ImportPixel16Swap();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
	unsigned TWO temp;
};

class BC_ImportPixel24Swap : public BC_ImportPixelBase
{
public:
	BC_ImportPixel24Swap();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

class BC_ImportPixel32Swap : public BC_ImportPixelBase
{
public:
	BC_ImportPixel32Swap();
	int import_pixel(BC_CapturePixel &output, unsigned char **input);
};

#endif
