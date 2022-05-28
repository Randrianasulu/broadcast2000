#include <string.h>
#include "audioconfig.h"
#include "bcmeter.inc"
#include "defaults.h"
#include "filesystem.h"
#include "preferences.h"
#include "videoconfig.h"
#include "videodevice.inc"

#include <stdlib.h>

#define CLAMP(x, y, z) (x) = ((x) < (y) ? (y) : ((x) > (z) ? (z) : (x)))

Preferences::Preferences()
{
	aconfig = new AudioConfig;
	vconfig = new VideoConfig;
}

Preferences::~Preferences()
{
	delete aconfig;
	delete vconfig;
}


int Preferences::calculate_smp()
{
/* Get processor count */
	int result = 1;
	FILE *proc;
	if(proc = fopen("/proc/cpuinfo", "r"))
	{
		char string[1024];
		while(!feof(proc))
		{
			fgets(string, 1024, proc);
			if(!strncasecmp(string, "processor", 9))
			{
				char *ptr = strchr(string, ':');
				if(ptr)
				{
					ptr++;
					result = atol(ptr) + 1;
				}
			}
			else
			if(!strncasecmp(string, "cpus detected", 13))
			{
				char *ptr = strchr(string, ':');
				if(ptr)
				{
					ptr++;
					result = atol(ptr);
				}
			}
		}
		fclose(proc);
	}
	printf("%i processors detected\n", result);
	return result - 1;
}


Preferences& Preferences::operator=(Preferences &that)
{
// ================================= Performance ================================
	strcpy(index_directory, that.index_directory);
	index_size = that.index_size;
	index_count = that.index_count;
	cache_size = that.cache_size;
	smp = that.smp;

// ==================================== audio ==================================
	*aconfig = *that.aconfig;

// Recording
	enable_duplex = that.enable_duplex;
	min_meter_db = that.min_meter_db;
	record_write_length = that.record_write_length;
	real_time_record = that.real_time_record;
	record_speed = that.record_speed;

// Playback
	view_follows_playback = that.view_follows_playback;
	real_time_playback = that.real_time_playback;
	playback_preload = that.playback_preload;
	playback_buffer = that.playback_buffer;
	playback_software_timer = that.playback_software_timer;
	audio_read_length = that.audio_read_length;
	audio_module_fragment = that.audio_module_fragment;
	test_playback_edits = that.test_playback_edits;
	scrub_speed = that.scrub_speed;

// ===================================== video =================================
	*vconfig = *that.vconfig;

	video_every_frame = that.video_every_frame;  
	video_output_length = that.video_output_length;  
	video_read_length = that.video_read_length;    
	video_write_length = that.video_write_length;   
	actual_frame_rate = that.actual_frame_rate;
	video_floatingpoint = that.video_floatingpoint;  
	video_use_alpha = that.video_use_alpha;   
	video_interpolate = that.video_interpolate;   

// ===================================== Interface =============================
	time_format = that.time_format;
	meter_format = that.meter_format;
	edit_handle_mode[0] = that.edit_handle_mode[0];           
	edit_handle_mode[1] = that.edit_handle_mode[1];           
	edit_handle_mode[2] = that.edit_handle_mode[2];           
	frames_per_foot = that.frames_per_foot;

// ====================================== Plugins ==============================
	strcpy(global_plugin_dir, that.global_plugin_dir);
	strcpy(local_plugin_dir, that.local_plugin_dir);

// Check boundaries
	CLAMP(video_write_length, 1, 1000);
	CLAMP(min_meter_db, -100, -1);
	CLAMP(cache_size, 1, 100);
	CLAMP(frames_per_foot, 1, 32);

	FileSystem fs;
	if(strlen(index_directory))
	{
		fs.complete_path(index_directory);
		fs.add_end_slash(index_directory);
	}
	
	if(strlen(global_plugin_dir))
	{
		fs.complete_path(global_plugin_dir);
		fs.add_end_slash(global_plugin_dir);
	}
	
	if(strlen(local_plugin_dir))
	{
		fs.complete_path(local_plugin_dir);
		fs.add_end_slash(local_plugin_dir);
	}
return *this;
}

int Preferences::load_defaults(Defaults *defaults)
{
	FileSystem fs;
	sprintf(index_directory, BCASTDIR);
	if(strlen(index_directory))
		fs.complete_path(index_directory);
	defaults->get("INDEX_DIRECTORY", index_directory);
	index_size = defaults->get("INDEX_SIZE", 1000000);
	index_count = defaults->get("INDEX_COUNT", 20);
	cache_size = defaults->get("CACHE_SIZE", 5);
	smp = calculate_smp();

	aconfig->load_defaults(defaults);

	enable_duplex = defaults->get("ENABLE_DUPLEX", 1);
	min_meter_db = defaults->get("MIN_RECORD_DB", -40);
	record_write_length = defaults->get("RECORD_WRITE_LENGTH", 131072); // Heroine kernel 2.2 scheduling sucks.
	real_time_record = defaults->get("REALTIME_RECORD", 0);
	record_speed = defaults->get("RECORD_SPEED", 8);  // Full lockup on anything higher

	view_follows_playback = defaults->get("VIEW_FOLLOWS_PLAYBACK", 1);
	real_time_playback = defaults->get("PLAYBACK_REALTIME", 0);
	playback_preload = defaults->get("PLAYBACK_PRELOAD", 0);
	playback_buffer = defaults->get("PLAYBACK_BUFFER", 4096);
	playback_software_timer = defaults->get("PLAYBACK_SOFTWARE_TIMER", 1);
	audio_read_length = defaults->get("PLAYBACK_READ_LENGTH", 131072);
	audio_module_fragment = defaults->get("AUDIO_MODULE_FRAGMENT", 4096);
	scrub_speed = defaults->get("SCRUB_SPEED", (float)2);
	test_playback_edits = defaults->get("TEST_PLAYBACK_EDITS", 1);

	vconfig->load_defaults(defaults);

	video_every_frame = defaults->get("VIDEO_EVERY_FRAME", 0);
	video_read_length = 1;    
	video_write_length = defaults->get("VIDEO_WRITE_LENGTH", 10);
	actual_frame_rate = defaults->get("ACTUAL_FRAME_RATE", (float)-1);
	video_floatingpoint = defaults->get("VIDEO_FLOATINGPOINT", 0);
	video_use_alpha = defaults->get("VIDEO_USE_ALPHA", 1);
	video_interpolate = defaults->get("VIDEO_INTERPOLATE", 0);

	time_format = defaults->get("TIME_FORMAT", 0);
	meter_format = defaults->get("METER_FORMAT", METER_DB);
	min_meter_db = defaults->get("MIN_METER_DB", -40);
	edit_handle_mode[0] = defaults->get("EDIT_HANDLE_MODE0", MOVE_ALL_EDITS);
	edit_handle_mode[1] = defaults->get("EDIT_HANDLE_MODE1", MOVE_ONE_EDIT);
	edit_handle_mode[2] = defaults->get("EDIT_HANDLE_MODE2", MOVE_NO_EDITS);
	frames_per_foot = defaults->get("FRAMES_PER_FOOT", (float)16);

	sprintf(global_plugin_dir, "");
	sprintf(local_plugin_dir, "");
	defaults->get("GLOBAL_PLUGIN_DIR", global_plugin_dir);
	defaults->get("LOCAL_PLUGIN_DIR", local_plugin_dir);

	return 0;
return 0;
}

int Preferences::save_defaults(Defaults *defaults)
{
	defaults->update("INDEX_DIRECTORY", index_directory);
	defaults->update("INDEX_SIZE", index_size);
	defaults->update("INDEX_COUNT", index_count);
	defaults->update("CACHE_SIZE", cache_size);

	aconfig->save_defaults(defaults);

	defaults->update("ENABLE_DUPLEX", enable_duplex);
	defaults->update("MIN_RECORD_DB", min_meter_db);
	defaults->update("RECORD_WRITE_LENGTH", record_write_length); // Heroine kernel 2.2 scheduling sucks.
	defaults->update("REALTIME_RECORD", real_time_record);
	defaults->update("RECORD_SPEED", record_speed);  // Full lockup on anything higher

	defaults->update("VIEW_FOLLOWS_PLAYBACK", view_follows_playback);
	defaults->update("PLAYBACK_REALTIME", real_time_playback);
	defaults->update("PLAYBACK_PRELOAD", playback_preload);
	defaults->update("PLAYBACK_BUFFER", playback_buffer);
	defaults->update("PLAYBACK_SOFTWARE_TIMER", playback_software_timer);
	defaults->update("PLAYBACK_READ_LENGTH", audio_read_length);
	defaults->update("AUDIO_MODULE_FRAGMENT", audio_module_fragment);
	defaults->update("SCRUB_SPEED", scrub_speed);
	defaults->update("TEST_PLAYBACK_EDITS", test_playback_edits);

	vconfig->save_defaults(defaults);

	defaults->update("VIDEO_EVERY_FRAME", video_every_frame);
	defaults->update("VIDEO_WRITE_LENGTH", video_write_length);
	defaults->update("ACTUAL_FRAME_RATE", actual_frame_rate);
	defaults->update("VIDEO_FLOATINGPOINT", video_floatingpoint);
	defaults->update("VIDEO_USE_ALPHA", video_use_alpha);
	defaults->update("VIDEO_INTERPOLATE", video_interpolate);

	defaults->update("TIME_FORMAT", time_format);
	defaults->update("METER_FORMAT", meter_format);
	defaults->update("MIN_METER_DB", min_meter_db);
	defaults->update("EDIT_HANDLE_MODE0", edit_handle_mode[0]);
	defaults->update("EDIT_HANDLE_MODE1", edit_handle_mode[1]);
	defaults->update("EDIT_HANDLE_MODE2", edit_handle_mode[2]);
	defaults->update("FRAMES_PER_FOOT", frames_per_foot);

	defaults->update("GLOBAL_PLUGIN_DIR", global_plugin_dir);
	defaults->update("LOCAL_PLUGIN_DIR", local_plugin_dir);
	return 0;
return 0;
}





