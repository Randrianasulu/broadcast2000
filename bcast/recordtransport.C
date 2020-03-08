#include <string.h>
#include "file.h"
#include "record.h"
#include "recordengine.h"
#include "recordtransport.h"


RecordGUITransport::RecordGUITransport(Record *record, RecordEngine *engine, BC_WindowBase *window)
{
	this->record = record;
	this->engine = engine;
	this->window = window;
}

RecordGUITransport::~RecordGUITransport()
{
}

int RecordGUITransport::create_objects(int x, int y, int h)
{
	window->add_tool(rewind_button = new RecordGUIRewind(record, engine, x, y, h));
	x += rewind_button->get_w();
	window->add_tool(back_button = new RecordGUIBack(record, engine, x, y, h));
	x += back_button->get_w();
	window->add_tool(duplex_button = new RecordGUIDuplex(engine, x, y, h));
	x += duplex_button->get_w();
	window->add_tool(record_button = new RecordGUIRec(engine, x, y, h));
	x += record_button->get_w();

	if(!record->do_audio)
	{
		window->add_tool(record_frame = new RecordGUIRecFrame(engine, x, y, h));
		x += record_frame->get_w();
	}

	window->add_tool(play_button = new RecordGUIPlay(engine, x, y, h));
	x += play_button->get_w();
	window->add_tool(stop_button = new RecordGUIStop(engine, x, y, h));
	x += stop_button->get_w();
	window->add_tool(fwd_button = new RecordGUIFwd(record, engine, x, y, h));
	x += fwd_button->get_w();
	window->add_tool(end_button = new RecordGUIEnd(record, engine, x, y, h));
	x += end_button->get_w();
	x_end = x + 10;
return 0;
}

int RecordGUITransport::keypress_event()
{
	if(window->get_keypress() == ' ')
	{
		if(engine->is_saving || engine->is_previewing)
		{
			window->unlock_window();
			engine->stop_operation();
			window->lock_window();
		}
		else
		{
			window->unlock_window();
			engine->start_saving();
			window->lock_window();
		}
		return 1;
	}
	return 0;
return 0;
}


RecordGUIRec::RecordGUIRec(RecordEngine *engine, int x, int y, int h)
 : BC_RecButton(x, y, h, h)
{ this->engine = engine; }

RecordGUIRec::~RecordGUIRec()
{
}

int RecordGUIRec::handle_event()
{
	unlock_window();
	engine->start_saving();
	lock_window();
return 0;
}

int RecordGUIRec::keypress_event()
{
	return 0;
return 0;
}

RecordGUIRecFrame::RecordGUIRecFrame(RecordEngine *engine, int x, int y, int h)
 : BC_FrameRecButton(x, y, h, h)
{ this->engine = engine; }

RecordGUIRecFrame::~RecordGUIRecFrame()
{
}

int RecordGUIRecFrame::handle_event()
{
	unlock_window();
	engine->save_frame();
	lock_window();
return 0;
}

int RecordGUIRecFrame::keypress_event()
{
	return 0;
return 0;
}

RecordGUIPlay::RecordGUIPlay(RecordEngine *engine, int x, int y, int h)
 : BC_ForwardButton(x, y, h, h)
{ this->engine = engine; }

RecordGUIPlay::~RecordGUIPlay()
{
}

int RecordGUIPlay::handle_event()
{
	unlock_window();
	engine->start_preview();
	lock_window();
return 0;
}

int RecordGUIPlay::keypress_event()
{
	return 0;
return 0;
}


RecordGUIStop::RecordGUIStop(RecordEngine *engine, int x, int y, int h)
 : BC_StopButton(x, y, h, h)
{ this->engine = engine; }

RecordGUIStop::~RecordGUIStop()
{
}

int RecordGUIStop::handle_event()
{
	unlock_window();
	engine->stop_operation();
	lock_window();
return 0;
}

int RecordGUIStop::keypress_event()
{
	return 0;
return 0;
}



RecordGUIRewind::RecordGUIRewind(Record *record, RecordEngine *engine, int x, int y, int h)
 : BC_RewindButton(x, y, h, h)
{ this->engine = engine; this->record = record; }

RecordGUIRewind::~RecordGUIRewind()
{
}

int RecordGUIRewind::handle_event()
{
	if((record->do_audio && engine->file->get_audio_length() > 0) ||
		(record->do_video && engine->file->get_video_length(record->get_frame_rate()) > 0))
	{
		unlock_window();
		engine->stop_operation();
		lock_window();
// start the engine over
		engine->file->seek_start();
		engine->update_position(0);
	}
return 0;
}

int RecordGUIRewind::keypress_event()
{
	return 0;
return 0;
}



RecordGUIBack::RecordGUIBack(Record *record, RecordEngine *engine, int x, int y, int h)
 : BC_FastReverseButton(x, y, h, h)
{ this->engine = engine; this->record = record; }

RecordGUIBack::~RecordGUIBack()
{
}

int RecordGUIBack::handle_event()
{
return 0;
}

int RecordGUIBack::button_press()
{
	unlock_window();
	engine->stop_operation();
	lock_window();

	engine->reset_current_delay();
	set_repeat(engine->get_current_delay());
	count = 0;
	return 1;
return 0;
}

int RecordGUIBack::button_release()
{
	unset_repeat();
	if(!count) engine->goto_prev_label();
	return 1;
return 0;
}

int RecordGUIBack::repeat()
{
	static long jump;

	count++;

	set_repeat(engine->get_current_delay());

	jump = engine->current_position - record->get_samplerate();

	if(jump < 0) jump = 0;
	engine->update_position(jump);
	if(record->do_audio) engine->file->set_audio_position(engine->current_position);
	if(record->do_video) engine->file->set_video_position(toframes(engine->current_position, record->get_samplerate(), record->get_framerate()), record->get_framerate());
	return 1;        // trap it
return 0;
}

int RecordGUIBack::keypress_event()
{
	return 0;
return 0;
}



RecordGUIFwd::RecordGUIFwd(Record *record, RecordEngine *engine, int x, int y, int h)
 : BC_FastForwardButton(x, y, h, h)
{ this->engine = engine; this->record = record; }

RecordGUIFwd::~RecordGUIFwd()
{
}

int RecordGUIFwd::handle_event()
{
return 0;
}

int RecordGUIFwd::button_press()
{
	unlock_window();
	engine->stop_operation();
	lock_window();

	engine->reset_current_delay();
	set_repeat(engine->get_current_delay());
	count = 0;
	return 1;
return 0;
}

int RecordGUIFwd::button_release()
{
	unset_repeat();
	if(!count) engine->goto_next_label();
return 0;
}

int RecordGUIFwd::repeat()
{
	static long jump;
	
	count++;
	
	set_repeat(engine->get_current_delay());

	jump = engine->current_position + engine->get_samplerate();
	if(jump > engine->total_length) jump = engine->total_length;
	engine->update_position(jump);
	if(record->do_audio) engine->file->set_audio_position(engine->current_position);
	if(record->do_video) engine->file->set_video_position(toframes(engine->current_position, record->get_samplerate(), record->get_framerate()), record->get_framerate());
	return 1;         // trap it
return 0;
}

int RecordGUIFwd::keypress_event()
{
	return 0;
return 0;
}



RecordGUIEnd::RecordGUIEnd(Record *record, RecordEngine *engine, int x, int y, int h)
 : BC_EndButton(x, y, h, h)
{ this->engine = engine; this->record = record; }

RecordGUIEnd::~RecordGUIEnd()
{
}

int RecordGUIEnd::handle_event()
{
	if((record->do_audio && engine->file->get_audio_length() > 0) ||
		(record->do_video && engine->file->get_video_length(record->get_frame_rate()) > 0))
	{
		unlock_window();
		engine->stop_operation();
		lock_window();

		engine->file->seek_end();
		engine->update_position(engine->total_length);
	}
return 0;
}

int RecordGUIEnd::keypress_event()
{
	return 0;
return 0;
}


RecordGUIDuplex::RecordGUIDuplex(RecordEngine *engine, int x, int y, int h)
 : BC_DuplexButton(x, y, h + 20, h)
{ this->engine = engine; }

RecordGUIDuplex::~RecordGUIDuplex()
{
}
	
int RecordGUIDuplex::handle_event()
{
	unlock_window();
	engine->start_saving(1);
	lock_window();
return 0;
}
