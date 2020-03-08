#include <string.h>
#include "channelpicker.h"
#include "colormodels.h"
#include "mainwindow.h"
#include "record.h"
#include "recordgui.h"
#include "recordtransport.h"
#include "recvideowindow.h"
#include "videodevice.inc"
#include "vframe.h"

#define BC_RGB888       9

RecordVideoWindow::RecordVideoWindow(RecordEngine *engine, Record *record, RecordGUI *gui)
 : Thread(1)
{
	this->record = record;
	this->gui = gui;
	this->engine = engine;
	slippery = 0;
	thread = 0;
}

RecordVideoWindow::~RecordVideoWindow()
{
	if(thread)
	{
		thread->stop_playback();
		delete thread;
	}
	window->set_done(0);
	Thread::join();
	delete window;
}

int RecordVideoWindow::create_objects()
{
	int w, h;
	w = record->frame_w;
	h = record->frame_h;
	fix_size(w, h, record->video_window_w, record->get_aspect_ratio());
	window = new RecordVideoWindowGUI(record, this, w, h + get_buttonbar_height());
	window->create_objects();
	if(record->get_video_driver() == CAPTURE_LML ||
		record->get_video_driver() == CAPTURE_FIREWIRE)
	{
// Slippery preview
		slippery = 1;
		thread = new RecordVideoThread(record, this, window);
		thread->start_playback();
	}

	start();
return 0;
}

void RecordVideoWindow::run()
{
	window->canvas->start_video();
	window->run_window();
	window->canvas->stop_video();
	close_threads();
}

int RecordVideoWindow::close_threads()
{
	if(window->channel_picker) window->channel_picker->close_threads();
return 0;
}

int RecordVideoWindow::update(VFrame *vframe, int format)
{
	if(slippery && vframe->get_compressed_size() && 
		(format == CAPTURE_LML || format == CAPTURE_FIREWIRE))
	{
		return thread->write_frame(vframe);
	}
	else
	if(format != CAPTURE_LML)
	{
		window->lock_window();
		window->create_bitmap();

		if(vframe->get_color_model() == VFRAME_VPIXEL)
			window->bitmap->read_frame(vframe, 0, 0, record->frame_w, record->frame_h, 0);
		else
			window->bitmap->read_frame(vframe->get_rows(), 
				record->frame_w, 
				record->frame_h);

		window->canvas->draw_bitmap(window->bitmap, 1);
		window->unlock_window();
	}
	return 0;
return 0;
}

int RecordVideoWindow::get_buttonbar_height()
{
	return RECBUTTON_HEIGHT;
return 0;
}

int RecordVideoWindow::fix_size(int &w, int &h, int width_given, float aspect_ratio)
{
	w = width_given;
	h = (int)((float)width_given / aspect_ratio);
return 0;
}

float RecordVideoWindow::get_scale(int w)
{
	if(record->get_aspect_ratio() > (float)record->frame_w / record->frame_h)
	{
		return (float)w / ((float)record->frame_h * record->get_aspect_ratio());
	}
	else
	{
		return (float)w / record->frame_w;
	}
}

int RecordVideoWindow::get_canvas_height()
{
	return window->get_h() - get_buttonbar_height();
return 0;
}

int RecordVideoWindow::get_channel_x()
{
//	return 240;
	return 5;
return 0;
}

int RecordVideoWindow::get_channel_y()
{
	return 2;
return 0;
}










RecordVideoWindowGUI::RecordVideoWindowGUI(Record *record, 
	RecordVideoWindow *thread, 
	int w, 
	int h)
 : BC_Window(ICONNAME ": Video in", 
 			w, 
 			h, 
			10, 10, 1, 
			!thread->record->video_window_open)
{
	this->thread = thread;
	this->record = record;
	bitmap = 0;
	translating = 0;
	translate_zoom = 0;
	channel_picker = 0;
	reverse_interlace = 0;
}

RecordVideoWindowGUI::~RecordVideoWindowGUI()
{
	if(bitmap) delete bitmap;
	if(channel_picker) delete channel_picker;
}

int RecordVideoWindowGUI::create_objects()
{
	int x = 5;
	channel_picker = 0;
	reverse_interlace = 0;
	add_tool(canvas = new BC_Canvas(0, thread->get_buttonbar_height(), get_w(), thread->get_canvas_height(), BLACK));
	add_subwindow(buttonbar = new BC_SubWindow(0, 0, get_w(), thread->get_buttonbar_height()));
	buttonbar->add_border();
	if(record->get_video_driver() == VIDEO4LINUX)
	{
		channel_picker = new ChannelPicker(thread->engine, 
			&(thread->record->mwindow->channeldb), 
			buttonbar);
		channel_picker->create_objects(thread->get_channel_x(), 
			thread->get_channel_y(), 
			thread->get_buttonbar_height());
		x += 130;
	}
	record_transport = new RecordGUITransport(thread->record, thread->engine, buttonbar);
	record_transport->create_objects(x, 2, thread->get_buttonbar_height() - 4);
	x = record_transport->x_end;
	if(record->get_video_driver() == CAPTURE_LML)
	{
		buttonbar->add_tool(reverse_interlace = new ReverseInterlace(record, x, 7));
	}

	set_title();
return 0;
}

int RecordVideoWindowGUI::button_press()
{
	if(get_buttonpress() == 2)
	{
// Middle button pressed
		if(shift_down())
		{
			translate_zoom = 1;
			get_virtual_center();
		}
		else
		{
			translating = 1;
			get_virtual_center();
		}
		return 1;
	}
	return 0;
return 0;
}

int RecordVideoWindowGUI::button_release()
{
	if(translating || translate_zoom)
	{
		translating = 0;
		translate_zoom = 0;
		return 1;
	}
	return 0;
return 0;
}

int RecordVideoWindowGUI::get_virtual_center()
{
	if(translate_zoom)
	{
		virtual_center_y = (int)(get_cursor_y() - record->video_zoom * record->frame_h);
	}
	else
	{
		virtual_center_x = get_cursor_x() - record->video_x;
		virtual_center_y = get_cursor_y() - record->video_y;
	}
return 0;
}

int RecordVideoWindowGUI::cursor_motion()
{
	if(translating || translate_zoom)
	{
		if(shift_down())
		{
			if(!translate_zoom)
			{
				translate_zoom = 1;
				translating = 0;
				get_virtual_center();
			}
		}
		else
		if(translate_zoom)
		{
			translate_zoom = 0;
			translating = 1;
			get_virtual_center();
		}

		if(translating)
		{
			record->video_x = get_cursor_x() - virtual_center_x;
			record->video_y = get_cursor_y() - virtual_center_y;
			thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
		}

		if(translate_zoom)
		{
			record->video_zoom = (float)(get_cursor_y() - virtual_center_y) / record->frame_h;
			thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
		}
		return 1;
	}
	return 0;
return 0;
}

int RecordVideoWindowGUI::keypress_event()
{
	int result = 0;
	switch(get_keypress())
	{
		case LEFT:
			if(!ctrl_down()) 
			{ 
				thread->gui->set_translation(--(record->video_x), record->video_y, record->video_zoom);
			}
			else
			{
				record->video_zoom -= 0.1;
				thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
			}
			result = 1;
			break;
		case RIGHT:
			if(!ctrl_down()) 
			{ 
				thread->gui->set_translation(++(record->video_x), record->video_y, record->video_zoom);
			}
			else
			{
				record->video_zoom += 0.1;
				thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
			}
			result = 1;
			break;
		case UP:
			if(!ctrl_down()) 
			{ 
				thread->gui->set_translation(record->video_x, --(record->video_y), record->video_zoom);
			}
			else
			{
				record->video_zoom -= 0.1;
				thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
			}
			result = 1;
			break;
		case DOWN:
			if(!ctrl_down()) 
			{ 
				thread->gui->set_translation(record->video_x, ++(record->video_y), record->video_zoom);
			}
			else
			{
				record->video_zoom += 0.1;
				thread->gui->set_translation(record->video_x, record->video_y, record->video_zoom);
			}
			result = 1;
			break;
		case 'w':
			close_event();
			break;
		default:
			result = record_transport->keypress_event();
			break;
	}
	return result;
return 0;
}

int RecordVideoWindowGUI::resize_event(int w, int h)
{
	float scale;
	int full_w, full_h;
	int new_w, new_h;

	new_w = w;
	new_h = h;
	if(record->get_aspect_ratio() > (float)record->frame_w / record->frame_h)
	{
		full_w = (int)((float)record->frame_h * record->get_aspect_ratio() + 0.5);
		full_h = record->frame_h;
	}
	else
	{
		full_w = record->frame_w;
		full_h = (int)((float)record->frame_w / record->get_aspect_ratio() + 0.5);
	}

	if(labs(full_w - new_w) < 50)
	{
		new_w = full_w;
		new_h = full_h;
	}
	else
		thread->fix_size(new_w, new_h, w, record->get_aspect_ratio());

	if(new_w < 10) new_w = 10;
	if(new_h < 10) new_h = 10;
	w = record->video_window_w = new_w;
	h = new_h;

	resize_window(new_w, new_h + thread->get_buttonbar_height());
	buttonbar->resize_window(0, 0, new_w, thread->get_buttonbar_height());
	canvas->set_size(0, thread->get_buttonbar_height(), new_w, new_h);
	set_title();
return 0;
}

int RecordVideoWindowGUI::set_title()
{
	char string[1024];
	int scale;

	scale = (int)(thread->get_scale(thread->record->video_window_w) * 100 + 0.5);

	sprintf(string, ICONNAME ": Video in %d%%", scale);
	BC_Window::set_title(string);
return 0;
}

int RecordVideoWindowGUI::close_event()
{
	thread->record->monitor_video = 0;
	thread->record->video_window_open = 0;
	thread->gui->monitor_video_toggle->update(0);
	hide_window();
return 0;
}

int RecordVideoWindowGUI::create_bitmap()
{
	if(bitmap && (bitmap->get_w() != get_w() || bitmap->get_h() != thread->get_canvas_height()))
	{
		delete bitmap;
		bitmap = 0;
	}

	if(!bitmap)
	{
		bitmap = canvas->new_bitmap(get_w(), thread->get_canvas_height());
	}
return 0;
}

ReverseInterlace::ReverseInterlace(Record *record, int x, int y)
 : BC_CheckBox(x, y, 16, 16, record->reverse_interlace, "Swap fields")
{
	this->record = record;
}

ReverseInterlace::~ReverseInterlace()
{
}

int ReverseInterlace::handle_event()
{
	record->reverse_interlace = get_value();
	return 0;
return 0;
}









// ================================== slippery playback ============================

METHODDEF(void)
recvideo_jpeg_init_source (j_decompress_ptr cinfo)
{
    recvideo_jpeg_src_ptr src = (recvideo_jpeg_src_ptr) cinfo->src;
}

METHODDEF(boolean)
recvideo_jpeg_fill_input_buffer (j_decompress_ptr cinfo)
{
	recvideo_jpeg_src_ptr src = (recvideo_jpeg_src_ptr) cinfo->src;

	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = 2;

	return TRUE;
}


METHODDEF(void)
recvideo_jpeg_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	recvideo_jpeg_src_ptr src = (recvideo_jpeg_src_ptr) cinfo->src;

	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
}


METHODDEF(void)
recvideo_jpeg_term_source (j_decompress_ptr cinfo)
{
}

GLOBAL(void)
recvideo_jpeg_buffer_src(j_decompress_ptr cinfo, unsigned char *buffer, long bytes)
{
	recvideo_jpeg_src_ptr src;

	if(cinfo->src == NULL) 
	{	/* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
    	(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof(recvideo_jpeg_source_mgr));
        src = (recvideo_jpeg_src_ptr) cinfo->src;
	}

	src = (recvideo_jpeg_src_ptr) cinfo->src;
	src->pub.init_source = recvideo_jpeg_init_source;
	src->pub.fill_input_buffer = recvideo_jpeg_fill_input_buffer;
	src->pub.skip_input_data = recvideo_jpeg_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = recvideo_jpeg_term_source;
	src->pub.bytes_in_buffer = bytes;
	src->pub.next_input_byte = buffer;
	src->buffer = buffer;
	src->bytes = bytes;
}

METHODDEF(void)
recvideo_jpeg_error_exit (j_common_ptr cinfo)
{
/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	recvideo_jpeg_error_ptr myerr = (recvideo_jpeg_error_ptr) cinfo->err;

/* Always display the message. */
/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}


RecordVideoThread::RecordVideoThread(Record *record, 
	RecordVideoWindow *device, 
	RecordVideoWindowGUI *window)
 : Thread()
{
	this->device = device;
	this->window = window;
	this->record = record;
	output_frame = new VFrame(0, record->frame_w, record->frame_h, VFRAME_RGB888);
	frame = new VFrame;
}

RecordVideoThread::~RecordVideoThread()
{
	delete output_frame;
	delete frame;
}

int RecordVideoThread::start_playback()
{
	synchronous = 1;
	ready = 1;
	done = 0;
	
	switch(record->get_video_driver())
	{
		case CAPTURE_LML:
			for(int i = 0; i < 2; i++)
			{
				jpeg_engine[i] = new RecVideoMJPGThread(record, this, window);
				jpeg_engine[i]->start_rendering();
			}
			break;
		
		case CAPTURE_FIREWIRE:
			dv_engine = new RecVideoDVEngine(record, this, window);
			dv_engine->start_rendering();
			break;
	}

	output_lock.lock();
	start();
	return 0;
return 0;
}

int RecordVideoThread::stop_playback()
{
	done = 1;
	output_lock.unlock();
	join();

	switch(record->get_video_driver())
	{
		case CAPTURE_LML:
			for(int i = 0; i < 2; i++)
			{
				jpeg_engine[i]->stop_rendering();
				delete jpeg_engine[i];
			}
			break;
		
		case CAPTURE_FIREWIRE:
			dv_engine->stop_rendering();
			delete dv_engine;
			break;
	}

	return 0;
return 0;
}

int RecordVideoThread::write_frame(VFrame *input_frame)
{
	if(ready)
	{
		ready = 0;
		frame->allocate_compressed_data(input_frame->get_compressed_size());
		memcpy(frame->get_data(), 
			input_frame->get_data(), 
			input_frame->get_compressed_size());
		frame->set_compressed_size(input_frame->get_compressed_size());
		output_lock.unlock();
	}
	return 0;
return 0;
}

int RecordVideoThread::render_jpeg()
{
	int i, j;
	long size = frame->get_compressed_size();
	unsigned char *data = (unsigned char*)frame->get_data();
	unsigned char *data1 = data + 1;
	int total_frames = 0;
	int frame2_offset;
	int field = 0;
	long field_size[2];

// Get offset of second frame
	for(i = 0, size--; i < size; i++)
	{
		if(data[i] == 0xff && data1[i] == 0xd8)
		{
			total_frames++;
			frame2_offset = i;
			if(total_frames > 1) break;
		}
	}
	size++;

	field_size[0] = frame2_offset;
	field_size[1] = size - frame2_offset;

// Decompress the frames
	for(i = 0; i < total_frames; )
	{
		for(j = 0; j < record->cpus && j + i < total_frames; j++)
		{
			jpeg_engine[j]->render_field(frame, (j + i) ? frame2_offset : 0, field_size[j + i], field);
			field ^= 1;
		}
		for(j = 0; j < record->cpus && j + i < total_frames; j++)
		{
			jpeg_engine[j]->wait_render();
		}
		i += j;
	}
	return 0;
return 0;
}

int RecordVideoThread::render_dv()
{
	dv_engine->render_frame(frame, frame->get_compressed_size());
	return 0;
return 0;
}

int RecordVideoThread::render_frame()
{
	switch(record->get_video_driver())
	{
		case CAPTURE_LML:
			render_jpeg();
			break;

		case CAPTURE_FIREWIRE:
			render_dv();
			break;
	}

	window->lock_window();
	window->create_bitmap();
	window->bitmap->read_frame(output_frame->get_rows(), 
		record->frame_w, 
		record->frame_h);
	window->canvas->draw_bitmap(window->bitmap, 1);
	window->unlock_window();
	return 0;
return 0;
}


void RecordVideoThread::run()
{
	while(!done)
	{
// Wait for next frame
		output_lock.lock();
		if(done) return;
		render_frame();
// Get next frame
		ready = 1;
	}
}



RecVideoMJPGThread::RecVideoMJPGThread(Record *record, RecordVideoThread *thread, RecordVideoWindowGUI *window)
 : Thread()
{
	Thread::synchronous = 1;
	this->record = record;
	this->thread = thread;
	this->window = window;
	create_jpeg_objects();
	input_lock.lock();
	output_lock.lock();
}

RecVideoMJPGThread::~RecVideoMJPGThread()
{
	delete_jpeg_objects();
}

int RecVideoMJPGThread::create_jpeg_objects()
{
	jpeg_decompress.err = jpeg_std_error(&(jpeg_error.pub));
	jpeg_error.pub.error_exit = recvideo_jpeg_error_exit;
	jpeg_create_decompress(&(jpeg_decompress));
return 0;
}

int RecVideoMJPGThread::delete_jpeg_objects()
{
	jpeg_destroy_decompress(&(jpeg_decompress));
return 0;
}

int RecVideoMJPGThread::start_rendering()
{
	done = 0;
	Thread::start();
	return 0;
return 0;
}

int RecVideoMJPGThread::stop_rendering()
{
	done = 1;
	input_lock.unlock();
	Thread::join();
	return 0;
return 0;
}


int RecVideoMJPGThread::render_field(VFrame *frame, long byte_offset, long size, int field)
{
	this->size = size;
	this->byte_offset = byte_offset;
	this->frame = frame;
	this->field = field;
	input_lock.unlock();
	return 0;
return 0;
}


int RecVideoMJPGThread::wait_render()
{
	output_lock.lock();
	return 0;
return 0;
}

void RecVideoMJPGThread::run()
{
	unsigned char *output_rows, *output_rows_end;
	int row_span = record->frame_w * 3;
	JSAMPROW row[1];
	int i;
	unsigned char *_601_to_rgb_table = record->_601_to_rgb_table;

	while(!done)
	{
		input_lock.lock();
		if(done) return;

		if(setjmp(jpeg_error.setjmp_buffer))
		{
// If we get here, the JPEG code has signaled an error.
			delete_jpeg_objects();
			create_jpeg_objects();
			output_lock.unlock();
			continue;
		}
		
		output_rows = thread->output_frame->get_data() + field * row_span;
		output_rows_end = output_rows + row_span * record->frame_h;
		row[0] = output_rows;
		recvideo_jpeg_buffer_src(&jpeg_decompress, (unsigned char*)frame->get_data() + byte_offset, size);
		jpeg_read_header(&jpeg_decompress, TRUE);
		jpeg_start_decompress(&jpeg_decompress);
		while(jpeg_decompress.output_scanline < jpeg_decompress.output_height &&
			row[0] < output_rows_end)
		{
			jpeg_read_scanlines(&jpeg_decompress, row, 1);
// Convert LML output to RGB
			for(i = 0; i < row_span; i++)
				row[0][i] = _601_to_rgb_table[row[0][i]];
			row[0] += row_span * 2;
		}
		jpeg_finish_decompress(&jpeg_decompress);
		output_lock.unlock();
	}
}


RecVideoDVEngine::RecVideoDVEngine(Record *record, RecordVideoThread *thread, RecordVideoWindowGUI *window)
{
	this->record = record;
	this->thread = thread;
	this->window = window;
	dv = 0;
}

RecVideoDVEngine::~RecVideoDVEngine()
{
}


int RecVideoDVEngine::start_rendering()
{
	dv = dv_new();
	return 0;
}

int RecVideoDVEngine::stop_rendering()
{
	dv_delete(dv);
	return 0;
}

int RecVideoDVEngine::render_frame(VFrame *frame, long size)
{
	dv_read_video(dv, 
		thread->output_frame->get_rows(), 
		frame->get_data(), 
		frame->get_compressed_size(),
		BC_RGB888);
	return 0;
}
