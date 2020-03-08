#ifndef AEDITS_H
#define AEDITS_H

#include "atrack.inc"
#include "edits.h"
#include "filehtal.inc"
#include "mainwindow.inc"

class AEdits : public Edits
{
public:
	AEdits() {printf("default edits constructor called\n");};
	AEdits(MainWindow *mwindow, ATrack *track);
	~AEdits() {};	


// ======================================= editing

	Edit* append_new_edit();
	Edit* insert_edit_after(Edit* previous_edit);
	int clone_derived(Edit* new_edit, Edit* old_edit);

// also known as paste_output
	int paste_edit(long start, 
				long end, 
				long startsource, 
				long length, 
				int channel, 
				Asset *asset);

	int identical(long sample1, long sample2);
private:
	ATrack *atrack;
};



#endif
