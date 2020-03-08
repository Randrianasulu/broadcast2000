#ifndef TRACK_H
#define TRACK_H

#include "arraylist.h"
#include "assets.inc"
#include "tracks.inc"
#include "autoconf.inc"
#include "datatype.h"
#include "edit.inc"
#include "edits.inc"
#include "filehtal.inc"
#include "floatautos.inc"
#include "linklist.h"
#include "mainwindow.inc"
#include "module.inc"
#include "patch.inc"
#include "toggleautos.inc"
#include "trackcanvas.inc"
#include "tracks.inc"
#include "transition.inc"

// UNITS ARE SAMPLES FOR ALL


class Track : public ListItem<Track>
{
public:
	Track() { };
	Track(MainWindow *mwindow, Tracks *tracks);
	virtual ~Track();

	int create_objects(int pixel, int flash);
	virtual int create_derived_objs(int flash) { return 0; };

	virtual int set_index_files(int flash, Asset *asset) { return 0; };

	int save(FileHTAL *htal);
	virtual int save_derived(FileHTAL *htal) { return 0; };
	int load(FileHTAL *htal, int track_offset, int automation_only, int edits_only);
	virtual int load_derived(FileHTAL *htal, int automation_only, int edits_only, int load_all, int &current_channel) { return 0; };
	int load_automation(FileHTAL *htal);
	int load_edits(FileHTAL *htal);

	virtual int change_channels(int oldchannels, int newchannels) { return 0; };
	virtual int dump();
	int render_init(int realtime_sched, int duplicate, long position);
	int render_stop(int duplicate);

// ==================================== drawing
	int draw(int x, int w, int y, int h, int flash = 1);
	virtual int draw_derived(int x, int w, int y, int h, int flash = 1) { return 0; };
	int draw_clear(int x, int w, int y, int h, int flash = 1);
	int draw_handles();   // position is y position of canvas
	int draw_titles();

	int draw_autos(AutoConf *auto_conf);
	int draw_floating_autos(AutoConf *auto_conf, int flash);
	virtual int draw_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf) { return 0; };
	virtual int draw_floating_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf, int flash) { return 0; };

	int track_visible(int x, int w, int y, int h);

// ===================================== editing
	int copy(long start, long end, FileHTAL *htal);
	int copy_assets(FileHTAL *htal, ArrayList<Asset*> *asset_list, long start, long end);
	virtual int copy_derived(long start, long end, FileHTAL *htal) { return 0; };
	int paste(long start, long end, long total_length, FileHTAL *htal);
	virtual int paste_derived(long start, long end, long total_length, FileHTAL *htal, int &current_channel) { return 0; };
	int paste_transition(long startproject, long endproject, Transition *transition);
	int clear(long start, long end, int convert_units = 1);
	virtual int clear_derived(long start, long end) { return 0; };

	int copy_automation(AutoConf *auto_conf, long selectionstart, long selectionend, FileHTAL *htal);
	virtual int copy_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, FileHTAL *htal) { return 0; };
	int paste_automation(long selectionstart, long selectionend, long total_length, FileHTAL *htal, int shift_autos = 1);
	virtual int paste_automation_derived(long selectionstart, long selectionend, long total_length, FileHTAL *htal, int shift_autos, int &current_pan) { return 0; };
	int clear_automation(AutoConf *auto_conf, long selectionstart, long selectionend, int shift_autos = 1);
	virtual int clear_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, int shift_autos = 1) { return 0; };
	int paste_auto_silence(long start, long end);
	virtual int paste_auto_silence_derived(long start, long end) { return 0; };
	int scale_time(float rate_scale, int scale_edits, int scale_autos, long start, long end);
	virtual int scale_time_derived(float rate_scale, int scale_edits, int scale_autos, long start, long end) { return 0; };
	int purge_asset(Asset *asset);
	int asset_used(Asset *asset);
	int clear_handle(long start, long end);
	int paste_silence(long start, long end);
	virtual int select_translation(int cursor_x, int cursor_y) { return 0; };  // select video coordinates for frame
	virtual int update_translation(int cursor_x, int cursor_y, int shift_down) { return 0; };  // move video coordinates
	int select_auto(AutoConf *auto_conf, int cursor_x, int cursor_y);
	virtual int select_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y) { return 0; };
	int move_auto(AutoConf *auto_conf, int cursor_x, int cursor_y, int shift_down);
	virtual int move_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y, int shift_down) { return 0; };
	int release_auto();
	virtual int release_auto_derived() { return 0; };
// Return whether automation would prevent direct frame copies.  Not fully implemented.
	int automation_is_used(long start, long end);
	virtual int automation_is_used_derived(long start, long end) { return 0; }

	int popup_transition(int cursor_x, int cursor_y);

// Return 1 if the left handle was selected 2 if the right handle was selected 3 if the track isn't recordable
	int select_handle(int cursor_x, int cursor_y, long &selection);
	int modify_handles(long oldposition, long newposition, int currentend, int handle_mode);
	int select_edit(long cursor_position, int cursor_x, int cursor_y, long &new_start, long &new_end);
	virtual int end_translation() { return 0; };
	virtual int reset_translation(long start, long end) { return 0; };
	int feather_edits(long start, long end, long units);
	long get_feather(long selectionstart, long selectionend);
	int swap_transitions(int number1, int number2);
	int shift_module_pointers(int deleted_track);

// ===================================== automation
	ToggleAutos *play_autos;
	ToggleAutos *mute_autos;
	FloatAutos *fade_autos;
// Transitions can't be supported yet because the position information
// for transitions would be relative to the transition and not the track,
// like automation must be.
	FloatAutos *plugin_autos[PLUGINS];

// ===================================== accounting
	int number_of();           // number of this track
	Patch* get_patch_of();
	Module* get_module_of();
	int data_type;     // TRACK_AUDIO or TRACK_VIDEO
	virtual long length();     // length of track in samples
// Test direct copy conditions common to all the rendering routines
	int direct_copy_possible(long start, long end);

// Samples_to_units is mostly used for determining a selection for editing so
// leave as int.
// converts the selection to SAMPLES OR FRAMES and stores in value
	virtual long samples_to_units(long &samples) { return 0; };
	int samples_to_units(long &start, long &end);    
	int samples_to_units(long &start, long &end, long &total_length);
// Convert the range to SAMPLES from whatever units the track is in and stores in the arguments
	virtual long units_to_samples(long &units);
	int units_to_samples(long &start, long &end);

// get_dimensions is used for getting drawing regions so use floats for partial frames
// get the display dimensions in SAMPLES OR FRAMES
	virtual int get_dimensions(float &view_start, float &view_units, float &zoom_units) { return 0; };   
// Position is in track's units
	Transition* get_transition(long position); 
// Compare last transition to current position
 	int test_transition(long current_position); 
// Longest time from current_position in which nothing changes
	long edit_change_duration(long input_position, long input_length, int reverse, int test_transitions);
// Utility for edit_change_duration.
	int edit_is_interesting(Edit *current, int test_transitions);
// If the edit under position is playable
	int playable_edit(long position);

// ===================================== for handles, titles, etc

	long old_view_start;
	int pixel;   // pixel position from top of track view
	Edits *edits;
	Tracks *tracks;
	MainWindow *mwindow;
// Transition currently being rendered
	Transition *transition;
};

#endif
