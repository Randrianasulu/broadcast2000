#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "audioconfig.inc"
#include "defaults.inc"
#include "preferences.inc"
#include "videoconfig.inc"

class Preferences
{
public:
	Preferences();
	~Preferences();
	
	Preferences& operator=(Preferences &that);
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);
	int calculate_smp();

// ================================= Performance ================================
// directory to look in for indexes
	char index_directory[1024];   
// size of index file in bytes
	long index_size;                  
	int index_count;
// Number of items to store in cache
	long cache_size;                  
// number of cpus to use - 1
	int smp;                          

// ==================================== audio ==================================
	AudioConfig *aconfig;

// Recording
	int enable_duplex;
	float min_meter_db;
	long record_write_length;
	int real_time_record;
	int record_speed;

// Playback
	int view_follows_playback;
	int real_time_playback;
	long playback_preload;
	long playback_buffer;
	int playback_software_timer;
	long audio_read_length;
	long audio_module_fragment;
	int test_playback_edits;
	float scrub_speed;

// ===================================== video =================================

	VideoConfig *vconfig;

// play every frame
	int video_every_frame;  
// number of frames to send through console and output at a time
	int video_output_length;  
// number of frames to read from disk during playback.
	int video_read_length;    
// number of frames to write to disk at a time during video recording.
	int video_write_length;   
	float actual_frame_rate;
// use floating point math for video
	int video_floatingpoint;  
// process alpha channels
	int video_use_alpha;   
// Interpolate video pixels when scaling
	int video_interpolate;   

// ===================================== Interface =============================
	int time_format;     // 0 HMS   1 HMSF
	int meter_format;
// Edit mode to use for each mouse button
	int edit_handle_mode[3];           
	float frames_per_foot;

// ====================================== Plugins ==============================
	char global_plugin_dir[1024];
	char local_plugin_dir[1024];
};

#endif
