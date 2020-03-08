#include <string.h>
#include "aedit.h"
#include "aedits.h"
#include "edits.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "patchbay.h"
#include "track.h"

AEdits::AEdits(MainWindow *mwindow, ATrack *track)
 : Edits(mwindow, (Track*)track)
{
	this->atrack = track;
}


Edit* AEdits::append_new_edit()
{
	AEdit *current;
	append(current = new AEdit(mwindow, this));
	return (Edit*)current;
}

Edit* AEdits::insert_edit_after(Edit* previous_edit)
{
	AEdit *current = new AEdit(mwindow, this);
	
	insert_after(previous_edit, current);

	return (Edit*)current;
}

int AEdits::clone_derived(Edit* new_edit, Edit* old_edit)
{
	((AEdit*)new_edit)->channel = ((AEdit *)old_edit)->channel;
return 0;
}

int AEdits::paste_edit(long start, 
				long end, 
				long startsource, 
				long length, 
				int channel, 
				Asset *asset)
{
	AEdit* current = (AEdit*)paste_edit_base(start, end, startsource, length, asset);

	current->channel = channel;

	optimize();
return 0;
}

int AEdits::identical(long sample1, long sample2)
{
// Interacting with video
	if(mwindow->patches->total_recordable_vtracks())
	{
		if(labs(sample1 - sample2) < mwindow->sample_rate / mwindow->frame_rate / 2)
    		return 1;
    	else
    		return 0;
	}
	else
// Audio only
	{
		if(labs(sample1 - sample2) < 2)
			return 1;
		else
			return 0;
	}
return 0;
}
