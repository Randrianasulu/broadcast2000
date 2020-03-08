#ifndef EDIT_H
#define EDIT_H

#include "assets.inc"
#include "bcbase.h"
#include "edits.inc"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "transition.inc"

// UNITS ARE SAMPLES FOR AUDIO / FRAMES FOR VIDEO
// zoom_units was mwindow->zoom_sample for AEdit

class Edit : public ListItem<Edit>
{
public:
	Edit(MainWindow *mwindow, Edits *edits);
	Edit() {};
	~Edit();

// ============================= initialization

	int load_properties(FileHTAL *htal, long &startproject);
	virtual int load_properties_derived(FileHTAL *htal) { return 0; };

// ============================= drawing

	virtual int draw(int flash, int center_pixel, int x, int w, int y, int h, int set_index_file) { return 0; };
	virtual int set_index_file(int flash, int center_pixel, int x, int y, int w, int h) { return 0; };
	int draw_transition(int flash, int center_pixel, int x, int w, int y, int h, int set_index_file);

	int draw_handles(BC_Canvas *canvas, float view_start, float view_units, float zoom_units, int view_pixels, int center_pixel);
	int draw_titles(BC_Canvas *canvas, float view_start, float zoom_units, int view_pixels, int center_pixel);

// ============================= editing

	int copy(long start, long end, FileHTAL *htal);
	virtual int copy_properties_derived(FileHTAL *htal, long length_in_selection) { return 0; };
	int paste(FileHTAL *htal);

	int popup_transition(float view_start, float zoom_units, int cursor_x, int cursor_y);

// Return 1 if the left handle was selected 2 if the right handle was selected
	int select_handle(float view_start, float zoom_units, int cursor_x, int cursor_y, long &selection);
	int shift_start_in(int edit_mode, long newposition, long oldposition);
	int shift_start_out(int edit_mode, long newposition, long oldposition);
	int shift_end_in(int edit_mode, long newposition, long oldposition);
	int shift_end_out(int edit_mode, long newposition, long oldposition);
	virtual int get_handle_parameters(long &left, long &right, long &left_sample, long &right_sample, float view_start, float zoom_units) { return 0; };
	virtual int long get_source_end() { return 0; };
	int dump();
	virtual int dump_derived() { return 0; };

// units are
// frames for video
// samples for audio
	long startsource;  // sample start of edit in source file
	long startproject;    // sample start of edit in project file
	long length;  // # of samples in edit
	long feather_left;         // feather left edit by # units
	long feather_right;         // feather right edit by # units

	Asset *asset;
	MainWindow *mwindow;
	Edits *edits;
	Transition *transition;    // Data for a transition if this edit is a transition
};





#endif
