#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "assets.inc"
#include "audiodevice.inc"
#include "bcbase.h"
#include "cache.inc"
#include "channel.inc"
#include "console.inc"
#include "defaults.inc"
#include "filehtal.inc"
#include "levelwindow.inc"
#include "mainundo.inc"
#include "mainwindow.inc"
#include "mainwindowgui.inc"
#include "maxchannels.h"
#include "patchbay.inc"
#include "playbackengine.inc"
#include "pluginserver.inc"
#include "preferences.inc"
#include "preferencesthread.inc"
#include "recordlabel.inc"
#include "threadloader.inc"
#include "timebar.inc"
#include "tracks.inc"
#include "transition.inc"
#include "videowindow.inc"


class MainWindow
{
public:
	MainWindow();
	~MainWindow();

// ======================================== initialization commands
	int create_objects(Defaults *defaults, 
		ArrayList<PluginServer*> *plugindb, 
		const char *local_plugin_dir, 
		const char *global_plugin_dir, 
		const char *display, 
		int want_gui, 
		int want_new);
	int run_script(FileHTAL *script);
	int new_project();
	int delete_project(int flash = 1);

	int load_defaults();
	int save_defaults();
	int set_filename(const char *filename);
	int load_channels();
	int save_channels();

// ========================================= synchronization

	int resize_lock;            // prevent tracks from redrawing on resize
	int lock_resize();
	int unlock_resize();


// ========================================= file operations

	int load_filenames(ArrayList<char*> *filenames);
	int load(const char *filename, int import_);                     // load a file
	int load(FileHTAL *htal, int import_ = 0,       // load from a htal file
		int edits_only = 0,
		int patches_only = 0,
		int console_only = 0,
		int timebar_only = 0,
		int automation_only = 0);
	int load_video_config(FileHTAL *htal, int import_);
	int load_audio_config(FileHTAL *htal, int import_,
		int edits_only, 
		int patches_only,
		int console_only,
		int automation_only);
	int save(FileHTAL *htal, int use_relative_path);       // save to a htal file
	int save_video_config(FileHTAL *htal);
	int save_audio_config(FileHTAL *htal);

	int load_edits(FileHTAL *htal);
	int load_patches(FileHTAL *htal);
	int load_console(FileHTAL *htal);
	int load_timebar(FileHTAL *htal);
	int load_automation(FileHTAL *htal);
	
	int interrupt_indexes();  // Stop index building

// ========================================== drawing

	int draw();          // draw everything

	int redraw_time_dependancies();     // after reconfiguring the time format, sample rate, frame rate

	int flip_vertical(int new_orientation);
// =========================================== movement

	int reposition_timebar(int new_pixel, int new_height);
	int expand_sample();
	int zoom_in_sample();
	int fit_sample();
	int move_left(long distance = 0);
	int move_right(long distance = 0);
	int next_label();   // seek to labels
	int prev_label();
	int samplemovement(long distance);
	int goto_start();
	int goto_end();
	int expand_y();
	int zoom_in_y();
	int expand_t();
	int zoom_in_t();
	int find_cursor();    // move the window to include the cursor
	int change_channels(int old_channels, int new_channels);

// ============================= editing commands ========================

	int cut(long start, long end);
	int paste_output(long startproject, 
				long endproject, 
				long startsource_sample, 
				long endsource_sample, 
				long startsource_frame,
				long endsource_frame,
				Asset *asset, 
				RecordLabels *new_labels);
	int paste_transition(long startproject, 
				long endproject, 
				Transition *transition);
	int clear(long start, long end);
	int feather_edits(long feather_samples, int audio, int video);
	long get_feather(int audio, int video);
	float get_aspect_ratio();
	int create_aspect_ratio(float &w, float &h, int width, int height);
	int mute_audio();
	int trim_selection();

	int cut_automation();
	int copy_automation();
	int paste_automation();
	int clear_automation();

	int paste_silence();
	int modify_handles(long oldposition, long newposition, int currentend, int handle_mode);

	int add_audio_track();
	int add_video_track();
	int delete_track();
	int toggle_label();

	int purge_asset(char *path);       // remove all traces of an asset from the project and cache
	int optimize_assets();            // delete unused assets from the cache and assets

// ================================= cursor selection ======================

	long align_to_frames(long sample);
// start a cursor selection
	int init_selection(long cursor_position, 
					int cursor_x, 
					int cursor_y, 
					int &current_end, 
					long &selection_midpoint1,
					long &selection_midpoint2,
					int &selection_type);
	int update_selection(long cursor_position,
					int cursor_x, 
					int cursor_y, 
					int &current_end, 
					long selection_midpoint1,
					long selection_midpoint2,
					int selection_type);
	int end_selection();
	int set_loop_boundaries();         // toggle loop playback and set boundaries for loop playback

// ================================ handle selection =======================

	int init_handle_selection(long cursor_position, int handle_pixel, int which_handle);     // handle selection
	int draw_floating_handle(int flash);          


// =========================================== windows

	MainWindowGUI *gui;
	LevelWindow *level_window;
	Console *console;
	Tracks *tracks;
	PatchBay *patches;
	MainUndo *undo;
	TimeBar *timebar;
	Defaults *defaults;
	Assets *assets;
	Cache *cache;
	ThreadLoader *threadloader;
	VideoWindow *video_window;
	Preferences *preferences;
	PreferencesThread *preferences_thread;

// ====================================== playback =============================

	int update_playback_cursor(long new_position, int view_follows_playback);
	int hide_playback_cursor(int flash = 1);
	int show_playback_cursor(long position  = -1, int flash = 1);
	int set_playback_range(long start_position = -1, int reverse = 0, float speed = 1);      // set the playback range for starting up
	int reset_meters();

// need playbutton to update when playback finishes
	int arm_playback(int follow_loop, 
					int use_buttons, 
					int infinite, 
					AudioDevice *audio);
	int pause_playback();                               // stop and arm playback
	int start_playback();                               // start instantly
	int wait_for_playback();							// when playing back from command line
	int stop_playback(int update_button = 0);           // stop
	int start_reconfigure(int unlock_window = 0);       // stop the playback for a reconfiguration
	int stop_reconfigure(int unlock_window = 0);        // restart playback after reconfiguration

	long get_playback_position();                   // get the exact position of the playback

	int is_playing_back;                            // flag for configuration
	int playback_cursor_visible;
	int loop_playback, loop_is_visible;
	long loop_start, loop_end;

	PlaybackEngine *playback_engine;
	long last_playback_position;

// ====================================== plugins ==============================

	ArrayList<PluginServer*> *plugindb;
	char *global_plugin_dir, *local_plugin_dir;
	int init_plugins();


// labels follow edits during editing
	int labels_follow_edits;     
 // automation follows edits during editing
 	int autos_follow_edits;     
// align cursor on frame boundaries
	int cursor_on_frames;         
// filename of the current project for window titling and saving
	char filename[1024];               
// for vertical tracks
	int tracks_vertical;               
// sample start of track view
	long view_start;                   
// pixel start of track view
	long track_start;                  
// zooming of the timeline
	long zoom_sample, zoom_y, zoom_track;       
// sample rate used by audio
	long sample_rate;                  
// frame rate used by video
	float frame_rate;                  

	int changes_made;
	int output_channels;
	int channel_positions[MAXCHANNELS];
// size of the compositing surface
	int track_w, track_h;              
// size of the video output
	int output_w, output_h;            
// Aspect ratio
	float aspect_w, aspect_h;          
// TV stations to record from
	ArrayList<Channel*> channeldb;     
// Adjust sample position to line up with frames.
	int fix_timing(long &samples_out, long &frames_out, long samples_in);     

// ===================================== formatting parameters

// top pixel of track view
	int get_top();                         
// bottom pixel of track view
	int get_bottom();                      
// Which X Server to use
	char display[1024];

// ================================== selection

	int set_selectionend(long new_position);
	int set_selectionstart(long new_position);
	int set_selection(long selectionstart, long selectionend);

// ==================================== cursor

	int get_affected_range(long *start, long *end, int reverse = 0);    // get range that rendering and playback affects
	long selectionstart, selectionend;       // selected range
};

#endif
