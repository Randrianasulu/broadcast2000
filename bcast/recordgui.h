#ifndef RECORDGUI_H
#define RECORDGUI_H

class RecordGUICancel;
class RecordGUIDCOffset;
class RecordGUILabel;
class RecordGUILoop;
class RecordGUILoopHr;
class RecordGUILoopMin;
class RecordGUILoopSec;
class RecordGUIModeMenu;
class RecordGUIMode;
class RecordGUIOK;
class RecordGUIReset;
class RecordGUIStartOver;
class RecordGUIMonitorVideo;
class RecordGUIMonitorAudio;

#include "bcbase.h"
#include "maxchannels.h"
#include "recordengine.inc"
#include "recordgui.inc"
#include "recordtransport.inc"
#include "record.inc"
#include "recvideowindow.inc"

class RecordGUI : public BC_Window
{
public:
	RecordGUI(Record *record, RecordEngine *engine, char *string, int height);
	~RecordGUI();

	int create_objects();

	RecordEngine *engine;
	RecordGUITransport *record_transport;
	RecordGUIOK *ok_button;
	RecordGUICancel *cancel_button;
	RecordGUIModeMenu *rec_mode_menu;
	RecordGUILoopHr *loop_hr;
	RecordGUILoopMin *loop_min;
	RecordGUILoopSec *loop_sec;
	RecordGUILabel *label_button;
	RecordGUIReset *reset;
	RecordGUIStartOver *startover_button;
	RecordGUIDCOffset *dc_offset_button;
	RecordGUIDCOffsetText *dc_offset_text[MAXCHANNELS];
	RecordGUIMonitorVideo *monitor_video_toggle;
	RecordGUIMonitorAudio *monitor_audio_toggle;
	RecordVideoWindow *monitor_video_window;
	BC_Meter *meter[MAXCHANNELS];
	BC_Title *position_title, *total_length_title;
	BC_Title *prev_label_title, *next_label_title;
	BC_Title *frames_dropped, *samples_clipped;
	long total_dropped_frames;
	long total_clipped_samples;



	char* get_path();
	int get_record_mode();
	int set_record_mode(int value);
	int get_output_bits();
	int get_dither();
	int get_duplex_status();
	int set_duplex_status(int value);
	int get_loop_status();
	int get_sample_rate();
	int get_enable_duplex();
	long get_playback_buffer();



	int set_loop_status(int value);
	int update_duration_boxes(); // Redraw the loop duration textboxes for a script.

	int keypress_event();
	int delete_all_labels();
	int calibrate_dc_offset();
	int calibrate_dc_offset(long new_value, int channel);
	int update_dropped_frames(long new_dropped);
	int update_clipped_samples(long new_clipped);
	int set_translation(int x, int y, float z);

	int update_position(long new_position);
	int update_total_length(long new_position);
	int update_prev_label(long new_position);
	int update_next_label(long new_position);

	int update_title(BC_Title *title, long position);

	int goto_prev_label();
	int goto_next_label();
	int toggle_label();
	Record *record;
};

class RecordGUIModeMenu : public BC_PopupMenu
{
public:
	RecordGUIModeMenu(int x, int y, int w, RecordEngine *engine, char *text);
	~RecordGUIModeMenu();

	int handle_event();
	int add_items();

	RecordGUIMode *linear;
	RecordGUIMode *timed;
	RecordGUIMode *loop;
	RecordEngine *engine;
};

class RecordGUIMode : public BC_PopupItem
{
public:
	RecordGUIMode(char *text);
	~RecordGUIMode();

	int handle_event();
};






class RecordGUILabel : public BC_BigButton
{
public:
	RecordGUILabel(RecordEngine *engine, int x);
	~RecordGUILabel();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIStartOver : public BC_BigButton, Thread
{
public:
	RecordGUIStartOver(RecordEngine *engine, int x);
	~RecordGUIStartOver();

	int handle_event();
	int keypress_event();
	void run();

	RecordEngine *engine;
};



class RecordGUILoopHr : public BC_TextBox
{
public:
	RecordGUILoopHr(RecordEngine *engine, int x, char *text);
	~RecordGUILoopHr();

	int handle_event();
	RecordEngine *engine;
};

class RecordGUILoopMin : public BC_TextBox
{
public:
	RecordGUILoopMin(RecordEngine *engine, int x, char *text);
	~RecordGUILoopMin();

	int handle_event();
	RecordEngine *engine;
};

class RecordGUILoopSec : public BC_TextBox
{
public:
	RecordGUILoopSec(RecordEngine *engine, int x, char *text);
	~RecordGUILoopSec();

	int handle_event();
	RecordEngine *engine;
};



class RecordGUIMonitorVideo : public BC_CheckBox
{
public:
	RecordGUIMonitorVideo(RecordEngine *engine, Record *record, RecordGUI *gui, int x, int y);
	~RecordGUIMonitorVideo();
	int handle_event();
	Record *record;
	RecordGUI *gui;
	RecordEngine *engine;
};

class RecordGUIMonitorAudio : public BC_CheckBox
{
public:
	RecordGUIMonitorAudio(RecordEngine *engine, Record *record, int x, int y);
	~RecordGUIMonitorAudio();
	int handle_event();
	Record *record;
	RecordEngine *engine;
};

class RecordGUIDCOffset : public BC_BigButton
{
public:
	RecordGUIDCOffset(RecordEngine *engine, int y);
	~RecordGUIDCOffset();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUIDCOffsetText : public BC_TextBox
{
public:
	RecordGUIDCOffsetText(RecordEngine *engine, char *text, int y, int number);
	~RecordGUIDCOffsetText();

	int handle_event();
	RecordEngine *engine;
	int number;
};

class RecordGUIReset : public BC_BigButton
{
public:
	RecordGUIReset(RecordGUI *gui, int y);
	~RecordGUIReset();

	int handle_event();
	RecordGUI *gui;
};

class RecordGUIResetTranslation : public BC_BigButton
{
public:
	RecordGUIResetTranslation(RecordGUI *gui, int y);
	~RecordGUIResetTranslation();

	int handle_event();
	RecordGUI *gui;
};



class RecordGUIOK : public BC_BigButton
{
public:
	RecordGUIOK(RecordEngine *engine, int y);
	~RecordGUIOK();

	int handle_event();
	int keypress_event();
	RecordEngine *engine;
};

class RecordGUICancel : public BC_BigButton, Thread
{
public:
	RecordGUICancel(RecordEngine *engine, int y);
	~RecordGUICancel();

	int handle_event();
	void run();
	int keypress_event();
	RecordEngine *engine;
};






#endif
