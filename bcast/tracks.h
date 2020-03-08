#ifndef TRACKS_H
#define TRACKS_H


#include "autoconf.h"
#include "cursor.inc"
#include "file.inc"
#include "filehtal.inc"
#include "linklist.h"
#include "mainwindow.inc"
#include "threadindexer.inc"
#include "track.h"
#include "trackcanvas.inc"
#include "transition.inc"



class Tracks : public List<Track>
{
public:
	Tracks(MainWindow *mwindow);
	virtual ~Tracks();

	int create_objects(Defaults *defaults, int w, int h, int top, int bottom);
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);

	int change_channels(int oldchannels, int newchannels);
	int set_index_file(int flash, Asset *asset);
	int dump();
	int vrender_init(int duplicate, long position);
	int arender_init(int realtime_sched, int duplicate, long position);
	int vrender_stop(int duplicate);
	int arender_stop(int duplicate);

// ======================================= file operations

	int load(FileHTAL *htal, int track_offset);
	int save(FileHTAL *htal);
	int update_old_filename(char *old_filename, char *new_filename);

// ======================================= drawing

	int draw(int flash = 1);            // draw everything visible
	int draw(int x, int w, int y, int h, int flash = 1);  // draw all visible tracks in the designated area

	int show_overlays(int flash);
	int hide_overlays(int flash);

	int resize_event(int w, int h, int top, int bottom);
	int flip_vertical(int top, int bottom);

	int draw_cursor(int flash);
	int draw_loop_points(int flash);

	int toggle_handles();
	int toggle_titles();
	int set_draw_output();

	int draw_handles(int flash);
	int draw_floating_handle(int flash);

	int draw_titles(int flash);

	int toggle_auto_fade();
	int toggle_auto_play();
	int toggle_auto_mute();
// set the video autos since only one can be visible at a time
	int set_show_autos(int camera, int project);
	int toggle_auto_project();
	int toggle_auto_project_z();
	int toggle_auto_camera();
	int toggle_auto_camera_z();
	int toggle_auto_pan(int pan);
	int toggle_auto_plugin(int plugin);
	int draw_autos(int flash);
	int show_autos(int flash);
	int hide_autos(int flash);
	int draw_floating_autos(int flash);
	int draw_playback_cursor(int x, int flash = 1);
	int draw_loop_points(long start, long end, int flash);


// ================================== movement

	int zoom_y();
	int expand_y();
	int expand_t();
	int zoom_t();
	int samplemovement(long distance);
	int trackmovement(long distance);
	int move_up(long distance = 0);
	int move_down(long distance = 0);
	int redo_pixels();           // reset all the track pixels after a delete or move

// ================================== track editing
	int import_audio_track(long length, int channel, Asset *asset);
	int import_video_track(long length, int layer, Asset *asset);
	int import_vtransition_track(long length, Asset *asset);
	int import_atransition_track(long length, Asset *asset);
	int add_audio_track(int flash = 1);      // add a track
	int add_video_track(int flash = 1);       // add a video track
	int add_vtransition_track(int flash = 1);
	int add_atransition_track(int flash = 1);

	int move_tracks_up();                   // move recordable tracks up
	int move_tracks_down();                 // move recordable tracks down
	int delete_track();     // delete last track
	int delete_audio_track();       // delete the last audio track
	int delete_video_track();        // delete the last video track
	int delete_track(Track* track);        // delete any track
	int delete_tracks();     // delete all the recordable tracks
	int delete_all(int flash = 1);      // delete just the tracks
	int concatenate_tracks();         // Append all the tracks to the end of the recordable tracks
	int copyable_tracks(long start, long end);  // return number of tracks to copy
	int swap_transitions(Track *track1, Track *track2);

// ================================== EDL editing
	int cut(long start, long end);
	int copy(long start, long end);
	int copy_assets(FileHTAL *htal, long start, long end, int all);
	int paste(long start, long end);
// all units are samples by default
	int paste_output(long startproject, 
				long endproject, 
				long startsource_sample, 
				long endsource_sample, 
				long startsource_frame, 
				long endsource_frame, 
				Asset *asset);
	int paste_transition(long startproject, 
				long endproject, 
				Transition *transition);
	int clear(long start, long end);
	int copy_automation(long selectionstart, long selectionend, FileHTAL *htal);
	int paste_automation(long selectionstart, long selectionend, long total_length, FileHTAL *htal);
	int clear_automation(long selectionstart, long selectionend);
	int clear_handle(long start, long end);
	int paste_silence(long start, long end);
	int purge_asset(Asset *asset);
	int asset_used(Asset *asset);
	int select_translation(int cursor_x, int cursor_y);    // select video coordinates for frame
	int update_translation(int cursor_x, int cursor_y, int shift_down);
// Transition popup
	int popup_transition(int cursor_x, int cursor_y);
// 1 if left handle selected 2 if right handle selected 3 if the track wasn't recordable
	int select_handle(int cursor_x, int cursor_y, long &handle_oldposition, long &handle_position, int &handle_pixel);
	int select_auto(int cursor_x, int cursor_y);
	int move_auto(int cursor_x, int cursor_y, int shift_down);
	int release_auto();
	int modify_handles(long oldposition, long newposition, int currentend, int handle_mode);
	int end_translation();
	int select_handles();
	int select_region();
	int select_edit(long cursor_position, int cursor_x, int cursor_y, long &new_start, long &new_end);
	int feather_edits(long start, long end, long samples, int audio, int video);
	long get_feather(long selectionstart, long selectionend, int audio, int video);
	int reset_translation(long start, long end);
	int scale_video(int *dimension, int *offsets, int scale_data);
// Move edit boundaries and automation during a framerate change
	int scale_time(float rate_scale, int ignore_record, int scale_edits, int scale_autos, long start, long end);

// ================================== accounting

	int handles, titles;               // show handles or titles
	int show_output;          // what type of video to draw
	AutoConf auto_conf;      // which autos are visible
	int overlays_visible;
	int view_start;         // vertical start of track view
	int view_pixels();          // return the view width in pixels from the canvas
	int vertical_pixels();       // return the view height in pixels
	long view_samples();      // return the view width in samples from the canvas
	long total_samples();     // return the longest track derived from the tracks
	long total_playable_samples();     // return the longest track derived from the playable tracks
	int totalpixels();       // height of all tracks in pixels
	int number_of(Track *track);        // track number of pointer
	Track* number(int number);      // pointer to track number
	int copy_length(long start, long end);

	MainWindow *mwindow;
	TrackCanvas *canvas;
	Cursor_ *cursor;

private:
	int paste_assets(FileHTAL *htal);
};

#endif
