#include <string.h>
#include "assets.h"
#include "edit.h"
#include "vedit.h"
#include "vedits.h"
#include "filesystem.h"
#include "mainwindow.h"
#include "loadfile.h"
#include "vtrack.h"

VEdits::VEdits(MainWindow *mwindow, VTrack *track) : Edits(mwindow, (Track*)track)
{
	this->vtrack = track;
}

Edit* VEdits::append_new_edit()
{
	VEdit *current;
	append(current = new VEdit(mwindow, this));
	return (Edit*)current;
}

Edit* VEdits::insert_edit_after(Edit* previous_edit)
{
	VEdit *current = new VEdit(mwindow, this);
	
	insert_after(previous_edit, current);

	return (Edit*)current;
}

int VEdits::clone_derived(Edit* new_edit, Edit* old_edit)
{
	VEdit *new_vedit = (VEdit*)new_edit;
	VEdit *old_vedit = (VEdit*)old_edit;

	new_vedit->layer = old_vedit->layer;
	new_vedit->center_x = old_vedit->center_x;
	new_vedit->center_y = old_vedit->center_y;
	new_vedit->center_z = old_vedit->center_z;
return 0;
}

int VEdits::paste_edit(long start, 
				long end, 
				long startsource, 
				long length, 
				int layer, 
				int center_x, 
				int center_y, 
				int center_z, 
				Asset *asset)
{
	VEdit* current = (VEdit*)paste_edit_base(start, end, startsource, length, asset);

	current->layer = layer;
	current->center_x = center_x;
	current->center_y = center_y;
	current->center_z = center_z;

	optimize();
return 0;
}


int VEdits::identical(long sample1, long sample2)
{
	if(labs(sample1 - sample2) <= 1) return 1; else return 0;
return 0;
}
