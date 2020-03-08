#include <string.h>
#include "file.h"
#include "recordengine.h"
#include "recconfirmdelete.h"
#include "recordgui.h"
#include "record.h"
#include "recordtransport.h"
#include "recordvideo.h"
#include "recvideowindow.h"
#include "videodevice.h"

RecordGUI::RecordGUI(Record *record, RecordEngine *engine, char *string, int height)
 : BC_Window("", MEGREY, string, 500, height, 500, height)
{
	this->engine = engine; 
	this->record = record;
}

RecordGUI::~RecordGUI()
{
	if(record->do_video)
	{
		delete monitor_video_window;
	}

	delete ok_button;
	delete cancel_button;
	delete loop_hr;
	delete loop_min;
	delete loop_sec;
	delete label_button;
	delete reset;
	delete startover_button;
	delete rec_mode_menu;

	if(record->do_audio)
	{
		delete dc_offset_button;
		for(int i = 0; i < engine->get_input_channels(); i++)
		{
			delete meter[i];
			delete dc_offset_text[i];
		}
	}
}

int RecordGUI::create_objects()
{
	File file;
// transport
	int x = 15, y, i;
	int column1 = 70, column2 = 200, column3 = 300, column4 = 400;
	monitor_video_toggle = 0;
	monitor_audio_toggle = 0;
	total_dropped_frames = 0;

	record_transport = new RecordGUITransport(record, engine, this);
	record_transport->create_objects(x, 15, 29);
	x = record_transport->x_end;
	add_tool(label_button = new RecordGUILabel(engine, x));	 x += 90;
	add_tool(startover_button = new RecordGUIStartOver(engine, x));	 x += 150;
	
// ================================ loop

	char string[256];
	add_tool(new BC_Title(5, 65, "Recording mode:"));
	engine->get_record_mode(string);
	add_tool(rec_mode_menu = new RecordGUIModeMenu(150, 60, 100, engine, string));

	x = 260;
	add_tool(new BC_Title(x, 65, "Duration:")); x += 80;
	add_tool(new BC_Title(x, 45, "Hours", SMALLFONT));
	sprintf(string, "%d", engine->get_loop_hr());
	add_tool(loop_hr = new RecordGUILoopHr(engine, x, string)); x += 45;
	add_tool(new BC_Title(x, 65, ":")); x += 10;
	add_tool(new BC_Title(x, 45, "Min", SMALLFONT));
	sprintf(string, "%d", engine->get_loop_min());
	add_tool(loop_min = new RecordGUILoopMin(engine, x, string)); x +=45;
	add_tool(new BC_Title(x, 65, ":")); x += 10;
	add_tool(new BC_Title(x, 45, "Sec", SMALLFONT));
	sprintf(string, "%d", engine->get_loop_sec());
	add_tool(loop_sec = new RecordGUILoopSec(engine, x, string)); x += 45;


	y = 95;
	
// ================================= format

	add_tool(new BC_Title(column1, y, "Format:"));
	engine->get_format(string);
	add_tool(new BC_Title(column2, y, string, MEDIUMFONT, MEYELLOW));

	if(record->do_audio)
	{
		add_tool(new BC_Title(column3, y, "Bits:"));
		add_tool(new BC_Title(column4, y, file.bitstostr(engine->get_bits()), MEDIUMFONT, MEYELLOW));
	}

	y += 20;
	if(record->do_audio)
	{
		add_tool(new BC_Title(column1, y, "Sample rate:"));
		sprintf(string, "%d", engine->get_samplerate());
		add_tool(new BC_Title(column2, y, string, MEDIUMFONT, MEYELLOW));
	}

	if(record->do_video)
	{
		add_tool(new BC_Title(column3, y, "Frame rate:"));
		sprintf(string, "%.2f", record->get_framerate());
		add_tool(new BC_Title(column4, y, string, MEDIUMFONT, MEYELLOW));
	}

// ================================= position/ last label

	y += 22;
	
	add_tool(new BC_Title(column1, y, "Position:"));
	add_tool(new BC_Title(column3, y, "Prev label:"));
	add_tool(position_title = new BC_Title(column2, y, "-", MEDIUMFONT, RED));
	add_tool(prev_label_title = new BC_Title(column4, y, "-", MEDIUMFONT, RED));
	
// ================================= end position/total labels

	y += 20;
	add_tool(new BC_Title(column1, y, "End:"));
	add_tool(new BC_Title(column3, y, "Next label:"));
	add_tool(total_length_title = new BC_Title(column2, y, "-", MEDIUMFONT, RED));
	add_tool(next_label_title = new BC_Title(column4, y, "-", MEDIUMFONT, RED));
	y += 20;

// ================================= dropped frames

	if(record->do_video)
	{
		add_tool(new BC_Title(column1, y, "Frames behind:"));
		add_tool(frames_dropped = new BC_Title(column2, y, "0", MEDIUMFONT, RED));
	}

// ================================= clipped samples

	if(record->do_audio)
	{
		add_tool(new BC_Title(column3, y, "Clipped samples:"));
		add_tool(samples_clipped = new BC_Title(column4 + 30, y, "0", MEDIUMFONT, RED));
	}

	y += 30;

	if(record->do_video)
	{
		add_tool(monitor_video_toggle = new RecordGUIMonitorVideo(engine, record, this, 5, y));
		add_tool(new BC_Title(150, y + 3, "Translation:"));
		add_tool(new RecordGUIResetTranslation(this, y));
	}

	y += 30;
// ==================================== VU METERS
	if(record->do_audio)
	{
		add_tool(monitor_audio_toggle = new RecordGUIMonitorAudio(engine, record, 5, y));
		add_tool(new BC_Title(150, y+3, "DC Offset:"));
		add_tool(dc_offset_button = new RecordGUIDCOffset(engine, y));

		add_tool(new BC_Title(340, y + 3, "Meters:"));
		add_tool(reset = new RecordGUIReset(this, y));

		y += 45;
		x = 100;

// print meters
		for(i = 0; i < engine->get_input_channels(); i++, y += 25)
		{
			sprintf(string, "%d:", i+1);
			add_tool(new BC_Title(5, y, string));

			sprintf(string, "%ld", engine->get_dc_offset(i));
			add_tool(dc_offset_text[i] = new RecordGUIDCOffsetText(engine, string, y, i));

			add_tool(meter[i] = new BC_Meter(100, y, 390, 20, 
				engine->get_min_db(), 
				engine->get_vu_format(), 
				engine->get_meter_over_hold(engine->get_meter_speed()), 
				engine->get_meter_peak_hold(engine->get_meter_speed())));
		}

		y -= 15 + 25 * engine->get_input_channels();

// print calibrated DB guage

		if(engine->get_vu_format() == METER_DB)
		{
			for(i = 0; i < 6; i++)
			{
				add_tool(new BC_Title((int)meter[0]->title_x[i] + x, y, meter[0]->db_titles[i], SMALLFONT));
			}
		}

		y += 25 * engine->get_input_channels();
	}

	y += 20;
	add_tool(ok_button = new RecordGUIOK(engine, y));
	add_tool(cancel_button = new RecordGUICancel(engine, y));

// Video monitor
	if(record->do_video)
	{
		monitor_video_window = new RecordVideoWindow(engine, record, this);
		monitor_video_window->create_objects();
	}
return 0;
}

int RecordGUI::set_translation(int x, int y, float z)
{
	record->video_x = x;
	record->video_y = y;
	record->video_zoom = z;

	//record->video_lock.lock();
	engine->vdevice->set_translation(x, y, z);
	engine->vdevice->set_latency_counter(record->get_video_buffersize() * 2);
	//record->video_lock.unlock();
return 0;
}

int RecordGUI::update_dropped_frames(long new_dropped)
{
	char string[1024];
	if(new_dropped < 0) new_dropped = 0;
	if(total_dropped_frames == new_dropped) return 0;

	total_dropped_frames = new_dropped;
	sprintf(string, "%d\n", total_dropped_frames);
	lock_window();
	frames_dropped->update(string);
	unlock_window();
	return 0;
return 0;
}

int RecordGUI::update_clipped_samples(long new_clipped)
{
	char string[1024];
	if(new_clipped < 0) new_clipped = 0;

	total_clipped_samples = new_clipped;
	sprintf(string, "%d\n", total_clipped_samples);
	lock_window();
	samples_clipped->update(string);
	unlock_window();
return 0;
}

int RecordGUI::keypress_event()
{
	return record_transport->keypress_event();
return 0;
}

int RecordGUI::update_position(long new_position) { update_title(position_title, new_position); return 0;
}

int RecordGUI::update_total_length(long new_position) { update_title(total_length_title, new_position); return 0;
}

int RecordGUI::update_prev_label(long new_position) { update_title(prev_label_title, new_position); return 0;
}

int RecordGUI::update_next_label(long new_position) { update_title(next_label_title, new_position); return 0;
}

int RecordGUI::update_title(BC_Title *title, long position)
{
	static char string[256];
	
	if(position != -1)
	{
		totext(string, 
				position, 
				engine->get_samplerate(), 
				engine->get_time_format(), 
				engine->get_frame_rate(), 
				engine->get_frames_per_foot());
	}
	else
	{
		sprintf(string, "-");
	}
	lock_window();
	title->update(string);
	unlock_window();
return 0;
}

int RecordGUI::update_duration_boxes()
{
	char string[1024];
	sprintf(string, "%d", engine->get_loop_hr());
	loop_hr->update(string);
	sprintf(string, "%d", engine->get_loop_min());
	loop_min->update(string);
	sprintf(string, "%d", engine->get_loop_sec());
	loop_sec->update(string);
return 0;
}






// ===================================== GUI


RecordGUILabel::RecordGUILabel(RecordEngine *engine, int x)
 : BC_BigButton(x, 5, "Label")
{ this->engine = engine; }


RecordGUILabel::~RecordGUILabel()
{
}

int RecordGUILabel::handle_event()
{
	engine->toggle_label();
return 0;
}

int RecordGUILabel::keypress_event()
{
	return 0;
return 0;
}

RecordGUIStartOver::RecordGUIStartOver(RecordEngine *engine, int x)
 : BC_BigButton(x, 5, "Start Over"), Thread()
{ this->engine = engine; }


RecordGUIStartOver::~RecordGUIStartOver()
{
}

int RecordGUIStartOver::handle_event()
{
	start();
return 0;
}

int RecordGUIStartOver::keypress_event()
{
	return 0;
return 0;
}

void RecordGUIStartOver::run()
{
	engine->start_over();
}




// ================================================== modes

RecordGUIModeMenu::RecordGUIModeMenu(int x, int y, int w, RecordEngine *engine, char *text)
 : BC_PopupMenu(x, y, w, text)
{ this->engine = engine; }

RecordGUIModeMenu::~RecordGUIModeMenu()
{
	delete linear;
	delete timed;
	delete loop;
}

int RecordGUIModeMenu::add_items()
{
	add_item(linear = new RecordGUIMode("Untimed"));
	add_item(timed = new RecordGUIMode("Timed"));
	add_item(loop = new RecordGUIMode("Loop"));
return 0;
}

int RecordGUIModeMenu::handle_event()
{
	engine->set_record_mode(text);
return 0;
}

RecordGUIMode::RecordGUIMode(char *text)
 : BC_PopupItem(text)
{
}

RecordGUIMode::~RecordGUIMode()
{
}
	
int RecordGUIMode::handle_event()
{
	get_menu()->update(text);
	get_menu()->handle_event();
return 0;
}










RecordGUILoopHr::RecordGUILoopHr(RecordEngine *engine, int x, char *text)
 : BC_TextBox(x, 60, 45, text)
{ this->engine = engine; }

RecordGUILoopHr::~RecordGUILoopHr()
{
}

int RecordGUILoopHr::handle_event()
{
	engine->set_loop_duration();
return 0;
}

RecordGUILoopMin::RecordGUILoopMin(RecordEngine *engine, int x, char *text)
 : BC_TextBox(x, 60, 45, text)
{ this->engine = engine; }

RecordGUILoopMin::~RecordGUILoopMin()
{
}

int RecordGUILoopMin::handle_event()
{
	engine->set_loop_duration();
return 0;
}

RecordGUILoopSec::RecordGUILoopSec(RecordEngine *engine, int x, char *text)
 : BC_TextBox(x, 60, 45, text)
{ this->engine = engine; }

RecordGUILoopSec::~RecordGUILoopSec()
{
}

int RecordGUILoopSec::handle_event()
{
	engine->set_loop_duration();
return 0;
}



RecordGUIMonitorVideo::RecordGUIMonitorVideo(RecordEngine *engine, Record *record, RecordGUI *gui, int x, int y)
 : BC_CheckBox(x, y, 16, 16, record->monitor_video, "Monitor video")
{
	this->engine = engine;
	this->record = record;
	this->gui = gui;
}

RecordGUIMonitorVideo::~RecordGUIMonitorVideo()
{
}

int RecordGUIMonitorVideo::handle_event()
{
// Video capture constitutively, just like audio, but only flash on screen if 1
	engine->set_monitor_video(get_value());
// 	if(!record->monitor_video && get_value() && !record->video_window_open)
// 	{
// 		gui->monitor_video_window->window->show_window();
// 	}
// 	record->monitor_video = get_value();
// 	if(get_value()) record->video_window_open = 1;
return 0;
}


RecordGUIMonitorAudio::RecordGUIMonitorAudio(RecordEngine *engine, Record *record, int x, int y)
 : BC_CheckBox(x, y, 16, 16, record->monitor_audio, "Monitor audio")
{
	this->record = record;
	this->engine = engine;;
}

RecordGUIMonitorAudio::~RecordGUIMonitorAudio()
{
}

int RecordGUIMonitorAudio::handle_event()
{
	engine->set_monitor_audio(get_value());
return 0;
}





RecordGUIDCOffset::RecordGUIDCOffset(RecordEngine *engine, int y)
 : BC_BigButton(230, y, "Calibrate")
{ this->engine = engine; }

RecordGUIDCOffset::~RecordGUIDCOffset() {}

int RecordGUIDCOffset::handle_event()
{
	engine->calibrate_dc_offset();
return 0;
}

int RecordGUIDCOffset::keypress_event() { return 0; return 0;
}

RecordGUIDCOffsetText::RecordGUIDCOffsetText(RecordEngine *engine, char *text, int y, int number)
 : BC_TextBox(30, y+1, 67, text, 0)
{ this->engine = engine; this->number = number; }

RecordGUIDCOffsetText::~RecordGUIDCOffsetText()
{
}
	
int RecordGUIDCOffsetText::handle_event()
{
	if(!engine->is_previewing)
	{
		engine->calibrate_dc_offset(atol(get_text()), number);
	}
return 0;
}

RecordGUIReset::RecordGUIReset(RecordGUI *gui, int y)
 : BC_BigButton(400, y, "Reset")
{ this->gui = gui; }

RecordGUIReset::~RecordGUIReset() 
{
}

int RecordGUIReset::handle_event()
{
	for(int i = 0; i < gui->engine->get_input_channels(); i++)
	{
		gui->meter[i]->reset_over();
	}
return 0;
}

RecordGUIResetTranslation::RecordGUIResetTranslation(RecordGUI *gui, int y)
 : BC_BigButton(250, y, "Reset")
{ this->gui = gui; }

RecordGUIResetTranslation::~RecordGUIResetTranslation() 
{
}

int RecordGUIResetTranslation::handle_event()
{
	gui->set_translation(0, 0, 1);
return 0;
}





RecordGUIOK::RecordGUIOK(RecordEngine *engine, int y)
 : BC_BigButton(50, y, "Save")
{ this->engine = engine; }

RecordGUIOK::~RecordGUIOK()
{
}

int RecordGUIOK::handle_event()
{
	unlock_window();
	engine->set_done(0);
	lock_window();
return 0;
}

int RecordGUIOK::keypress_event()
{
	return 0;
return 0;
}



RecordGUICancel::RecordGUICancel(RecordEngine *engine, int y)
 : BC_BigButton(320, y, "Cancel"), Thread()
{ this->engine = engine; }

RecordGUICancel::~RecordGUICancel()
{
}

int RecordGUICancel::handle_event()
{
	disable_window();
	start();
return 0;
}

void RecordGUICancel::run()
{
	int result = 0;
	if((engine->record->do_audio && engine->file->get_audio_length() > 0) ||
		(engine->record->do_video && engine->file->get_video_length(engine->record->get_frame_rate()) > 0))
	{
		RecConfirmDelete dialog;
		dialog.create_objects("cancel");
		result = dialog.run_window();
		enable_window();
	}
	if(!result)
	{
//		engine->stop_operation();
		engine->set_done(1);
	}
}

int RecordGUICancel::keypress_event()
{
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}
