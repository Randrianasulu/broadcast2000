#ifndef EDITS_H
#define EDITS_H


#include "assets.inc"
#include "bcbase.h"
#include "edit.inc"
#include "filehtal.inc"
#include "linklist.h"
#include "mainwindow.inc"
#include "track.inc"
#include "transition.inc"

// UNITS ARE SAMPLES FOR AUDIO  / FRAMES FOR VIDEO

class Edits : public List<Edit>
{
public:
// ============================= initialization commands ====================
	Edits() { printf("default edits constructor called\n"); };
	Edits(MainWindow *mwindow, Track *track);
	~Edits() { };	

// ================================== file operations

	int save(FileHTAL *htal);
	int load(FileHTAL *htal, int track_offset);
	int load_edit(FileHTAL *htal, long &startproject, int track_offset);

	virtual Edit* append_new_edit() { return 0; };
	virtual Edit* insert_edit_after(Edit *previous_edit) { return 0; };
	virtual int load_edit_properties(FileHTAL *htal) { return 0; };


// ==================================== accounting

	Edit* editof(long position);  // edit on or immediately after
// Return an edit if position is over an edit and the edit has a source file
	Edit* get_render_strategy(long position, ArrayList<int> *render_strategies, int &render_strategy);
	long total_length();
	long end();         // end position of last edit

// ==================================== editing

// inserts space at the desired location and returns the edit before the space
// fills end of track if range is after track
	Edit* insert(long start, long lengthsamples);
	int copy(long start, long end, FileHTAL *htal);
	int clear(long start, long end);
	int clear_handle(long start, long end);
	int modify_handles(long oldposition, long newposition, int currentend, int edit_mode);
	int paste(long start, long end, long total_length, FileHTAL *htal);
	int paste_silence(long start, long end);
	int paste_transition(long startproject, long endproject, Transition *transition);
	Edit* paste_edit_base(long start, long end, long startsource, long length, Asset *asset);
	int optimize();
// Positions are identical for handle modifications
    virtual int identical(long sample1, long sample2) { return 0; };

	MainWindow *mwindow;
	Track *track;

private:
	virtual int clone_derived(Edit* new_edit, Edit* old_edit) { return 0; };
};



#endif
