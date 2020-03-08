#ifndef VEDITS_H
#define VEDITS_H

#include "edits.h"
#include "filehtal.h"
#include "mainwindow.inc"
#include "vtrack.inc"



class VEdits : public Edits
{
public:
	VEdits() {printf("default edits constructor called\n");};
	VEdits(MainWindow *mwindow, VTrack *track);
	~VEdits() { };


// ========================================= editing

	Edit* append_new_edit();
	Edit* insert_edit_after(Edit* previous_edit);
	int clone_derived(Edit* new_edit, Edit* old_edit);

// also known as paste_output
	int paste_edit(long start, 
				long end, 
				long startsource, 
				long length, 
				int layer, 
				int center_x, 
				int center_y, 
				int center_z, 
				Asset *asset);

	int identical(long sample1, long sample2);
	VTrack *vtrack;
};


#endif
