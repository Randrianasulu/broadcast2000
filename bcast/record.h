#ifndef RECORD_H
#define RECORD_H

#include "assets.inc"
#include "bcbase.h"
#include "bitspopup.h"
#include "browsebutton.h"
#include "channel.inc"
#include "defaults.inc"
#include "file.inc"
#include "filehtal.inc"
#include "formatpopup.h"
#include "formattools.inc"
#include "mainwindow.inc"
#include "maxchannels.h"
#include "mutex.h"

class Record : public BC_RecButton, public Thread
{
public:
	Record(Defaults *defaults, MainWindow *mwindow, int x, int y);
	~Record();

	int handle_event();
	int keypress_event();
	int set_script(FileHTAL *script);
	int run_script(Asset *asset, int &do_audio, int &do_video);
	void run();
	int load_defaults(Asset *asset);
	int save_defaults(Asset *asset);
	int save_engine_defaults();
	int get_duplex_range(long *start, long *end);

	Channel *get_current_channel();
	float get_min_db();
	int get_samplerate();
	float get_framerate();
	int get_everyframe();
	int get_time_format();
	int get_realtime();
	float get_frame_rate();
	char* get_in_path();
	char* get_video_inpath();
	int get_video_driver();
	int get_vu_format();
	int get_rec_mode();
	int set_rec_mode(int value);
	int set_loop_duration(long value);
	int use_floatingpoint();
	float get_aspect_ratio();

	int get_out_length();   // Length to write during playback
	long get_out_buffersize();  // Same as get_out_length
	int get_software_positioning();
	long get_in_buffersize();      // Length to write to disk at a time
	int get_video_buffersize();    // Number of frames to write to disk at a time
	int get_video_capturesize();    // Number of frames to read from device at a time
	int get_meter_over_hold(int divisions);
	int get_meter_peak_hold(int divisions);
	int get_meter_speed();

	long get_playback_buffer();
	int enable_duplex();

	int in_progress;      // Recording is happening now
	int realtime;
	int to_tracks;
	int duplex_status;
	long loop_duration;
	long startsource_sample;           // start in source file of this recording
	long endsource_sample;
	long startsource_frame;
	long endsource_frame;
	int dither;
	long dc_offset[MAXCHANNELS];
	int append_to_file;
	int record_mode;
	int do_audio;
	int do_video;
	int monitor_audio;
	int monitor_video;
	int video_window_open;
	int frame_w;
	int frame_h;
	int video_x;
	int video_y;
	float video_zoom;
	int reverse_interlace;     // Reverse the interlace in the video window display only
	int video_window_w;       // Width of record video window
// Lock video device during resizes
	Mutex video_lock;
	int current_channel;      // Number of channeldb entry
	int video_brightness;     // Picture quality
	int video_hue;
	int video_color;
	int video_contrast;
	int video_whiteness;
	int cpus;

	FileHTAL *script;
	MainWindow *mwindow;
	Defaults *defaults;
// Table for LML conversion
	unsigned char _601_to_rgb_table[256];
};

class RecordOK;
class RecordCancel;
class RecordPath;
class RecordPathText;
class RecordChannels;
class RecordToTracks;
class RecordFormat;
class RecordBits;
class RecordSigned;
class RecordDither;
class RecordHILO;
class RecordLOHI;

class RecordWindow : public BC_Window
{
public:
	RecordWindow(Record *record, Asset *asset);
	~RecordWindow();

	int create_objects();


	RecordOK *ok_button;
	RecordCancel *cancel_button;
	RecordPath *path_button;
	RecordPathText *pathtext_button;
	RecordChannels *channels_button;
	RecordToTracks *to_tracks_button;
	RecordFormat *format_button;
	RecordBits *bits_button;
	RecordSigned *signed_button;
	RecordDither *dither_button;
	RecordHILO *hilo_button;
	RecordLOHI *lohi_button;
	FormatTools *format_tools;

	Record *record;
	Asset *asset;
};

class RecordOK : public BC_BigButton
{
public:
	RecordOK();
	~RecordOK();
	int handle_event();
	int keypress_event();
};

class RecordCancel : public BC_BigButton
{
public:
	RecordCancel();
	~RecordCancel();
	int handle_event();
	int keypress_event();
};

class RecordPath : public BrowseButton
{
public:
	RecordPath(Record *record, BC_TextBox *textbox, char *text);
	~RecordPath();

	Record *record;
};



class RecordPathText : public BC_TextBox
{
public:
	RecordPathText(Record *record, Asset *asset);
	~RecordPathText();
	int handle_event();

	Record *record;
	Asset *asset;
};


class RecordChannels : public BC_TextBox
{
public:
	RecordChannels(Record *record, Asset *asset);
	~RecordChannels();
	int handle_event();

	Record *record;
	Asset *asset;
};


class RecordDither : public BC_CheckBox
{
public:
	RecordDither(Record *record, int default_);
	~RecordDither();

	int handle_event();
	Record *record;
};

class RecordToTracks : public BC_CheckBox
{
public:
	RecordToTracks(Record *record, int default_);
	~RecordToTracks();

	int handle_event();
	Record *record;
};

class RecordFormat : public FormatPopup
{
public:
	RecordFormat(Record *record, Asset *asset, char* default_);
	~RecordFormat();
	
	int handle_event();
	Record *record;
	Asset *asset;
};

class RecordBits : public BitsPopup
{
public:
	RecordBits(Record *record, Asset *asset, char* default_);
	~RecordBits();
	
	int handle_event();
	Record *record;
	Asset *asset;
};

class RecordSigned : public BC_CheckBox
{
public:
	RecordSigned(Record *record, Asset *asset);
	~RecordSigned();
	
	int handle_event();
	Record *record;
	Asset *asset;
};

class RecordHILO : public BC_Radial
{
public:
	RecordHILO(Record *record, Asset *asset);
	~RecordHILO();
	
	int handle_event();
	Record *record;
	RecordLOHI *lohi;
	Asset *asset;
};

class RecordLOHI : public BC_Radial
{
public:
	RecordLOHI(Record *record, RecordHILO *hilo, Asset *asset);
	~RecordLOHI();
	
	int handle_event();
	Record *record;
	RecordHILO *hilo;
	Asset *asset;
};


#endif
