#ifndef PATCHBAY_H
#define PATCHBAY_H

#include "bcbase.h"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "patch.h"

class PatchBay : public List<Patch>
{
public:
	PatchBay(MainWindow *mwindow);
	~PatchBay();

	int create_objects(int top, int bottom);    // routine user adds components from
	int create_gui(int top, int bottom);

// ============================================ file operations

	int load_patches(FileHTAL *htal, Patch *current_patch);   // for undo

// =========================================== drawing

	int resize_event(int top, int bottom);
	int flip_vertical(int top, int bottom);

// =========================================== editing

// need the data type to get the proper title
	int add_track(int start_pixel, char *default_title, int data_type);
	int delete_track(int start_pixel);
	int delete_track(Patch *patch, int start_pixel);
	int delete_all();
	int expand_t(int start_pixel);
	int zoom_in_t(int start_pixel);
	int trackmovement(int distance);
	int redo_pixels(int start_pixel);


	int deselect_all_play();
	int select_all_play();
	int deselect_all_record();
	int select_all_record();
	int deselect_all_auto();
	int select_all_auto();
	int deselect_all_draw();
	int select_all_draw();
	int plays_selected();
	int records_selected();
	int autos_selected();
	int draws_selected();

// queries for getting title
	int total_audio();
	int total_video();
	int total_playable_atracks();
	int total_playable_vtracks();
	int total_recordable_tracks(int data_type);
	int total_recordable_atracks();
	int total_recordable_vtracks();
	int number_of(Patch *patch);        // patch number of pointer
	int copy_length();
	Patch* number(int number);      // pointer to patch number

	MainWindow *mwindow;
	PatchBayGUI *gui;

	BC_TextBox* atrack_title_number(int number);    // return textbox of atrack # to console 
	int button_down, new_status, reconfigure_trigger;
};

class PatchBayGUI : public BC_SubWindow
{
public:
	PatchBayGUI(MainWindow *mwindow, int x, int y, int w, int h);
	~PatchBayGUI();

	int cursor_motion();    // trap cursor motions for scrolling
	int button_release();
	MainWindow *mwindow;
	PatchBay *patches;
};

#endif
