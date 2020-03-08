#ifndef RECVIDEOWINDOW_H
#define RECVIDEOWINDOW_H

#include "bcbase.h"
#include "channelpicker.inc"
#include "libdv.h"
#include "jpegwrapper.h"
#include "record.inc"
#include "recordengine.inc"
#include "recordgui.inc"
#include "recordtransport.inc"
#include "recvideowindow.inc"

#include <png.h>

class RecordVideoThread;

struct recvideo_jpeg_error_mgr 
{
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct recvideo_jpeg_error_mgr* recvideo_jpeg_error_ptr;


typedef struct 
{
	struct jpeg_source_mgr pub;	/* public fields */

	JOCTET * buffer;		/* start of buffer */
	int bytes;             /* total size of buffer */
} recvideo_jpeg_source_mgr;

typedef recvideo_jpeg_source_mgr * recvideo_jpeg_src_ptr;

class RecordVideoWindow : public Thread
{
public:
	RecordVideoWindow(RecordEngine *engine, Record *record, RecordGUI *gui);
	~RecordVideoWindow();

	void run();

	int close_threads();   // Stop all the child threads on exit
	int create_objects();
	int update(VFrame *vframe, int format);  // Format is PLAYBACK_LML or PLAYBACK_X11
	int fix_size(int &w, int &h, int width_given, float aspect_ratio);
	float get_scale(int w);
	int get_buttonbar_height();
	int get_canvas_height();
	int get_channel_x();
	int get_channel_y();

	RecordGUI *gui;
	Record *record;
	RecordEngine *engine;
	RecordVideoWindowGUI *window;
	int slippery;
	int reverseinterlace;
	RecordVideoThread *thread; // Thread for slippery playback
};

class ReverseInterlace;

class RecordVideoWindowGUI : public BC_Window
{
public:
	RecordVideoWindowGUI(Record *record, 
		RecordVideoWindow *thread, 
		int w, int h);
	~RecordVideoWindowGUI();

	int create_objects();
	int resize_event(int w, int h);
	int set_title();
	int close_event();
	int create_bitmap();
	int button_press();
	int button_release();
	int cursor_motion();
	int get_virtual_center();
	int keypress_event();

	BC_SubWindow *buttonbar;
	BC_Canvas *canvas;
	BC_Bitmap *bitmap;
	RecordVideoWindow *thread;
	RecordGUITransport *record_transport;
	ChannelPicker *channel_picker;
	ReverseInterlace *reverse_interlace;
	Record *record;
	int translating;
	int translate_zoom;
	int virtual_center_x, virtual_center_y;
};

class ReverseInterlace : public BC_CheckBox
{
public:
	ReverseInterlace(Record *record, int x, int y);
	~ReverseInterlace();
	int handle_event();
	Record *record;
};

class RecVideoMJPGThread;
class RecVideoDVEngine;

class RecordVideoThread : public Thread
{
public:
	RecordVideoThread(Record *record, RecordVideoWindow *device, RecordVideoWindowGUI *window);
	~RecordVideoThread();

	void run();
	int start_playback();
	int stop_playback();
	int write_frame(VFrame *input_frame);
	int render_frame();

	VFrame *frame;    // Compressed frame being rendered
	VFrame *output_frame;  // Frame for the JPEG decompressor output
	Mutex output_lock;   // Wait for new frame
	RecordVideoWindow *device;
	RecordVideoWindowGUI *window;
	Record *record;

private:
	int render_jpeg();
	int render_dv();

	int ready;   // Ready to recieve the next frame
	int done;
	RecVideoMJPGThread *jpeg_engine[2];
	RecVideoDVEngine *dv_engine;
};

class RecVideoMJPGThread : public Thread
{
public:
	RecVideoMJPGThread(Record *record, RecordVideoThread *thread, RecordVideoWindowGUI *window);
	~RecVideoMJPGThread();

	int render_field(VFrame *frame, long byte_offset, long size, int field);
	int wait_render();
	int start_rendering();
	int stop_rendering();
	void run();

	Mutex input_lock, output_lock;
	RecordVideoWindowGUI *window;
	RecordVideoThread *thread;
	Record *record;

private:
	int create_jpeg_objects();
	int delete_jpeg_objects();

	struct jpeg_decompress_struct jpeg_decompress;
	struct recvideo_jpeg_error_mgr jpeg_error;
	int field;
	VFrame *frame;    // Compressed frame being rendered
	int done;
	long byte_offset;
	long size;
};

class RecVideoDVEngine
{
public:
	RecVideoDVEngine(Record *record, RecordVideoThread *thread, RecordVideoWindowGUI *window);
	~RecVideoDVEngine();

	int start_rendering();
	int stop_rendering();
	int render_frame(VFrame *frame, long size);

	RecordVideoWindowGUI *window;
	RecordVideoThread *thread;
	Record *record;

private:
	dv_t *dv;
};

#endif
